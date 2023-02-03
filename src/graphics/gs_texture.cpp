#include "gs_texture.h"
#include "graphics.h"

bool fbo_info::attach_rendertarget(std::shared_ptr<gs_texture> tex) {
    if (cur_render_target.lock() == tex)
        return true;

    cur_render_target = tex;

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex->gs_texture_obj(), 0);
    return gl_success("glFramebufferTexture2D");
}

bool fbo_info::attach_zstencil(std::shared_ptr<gs_zstencil_buffer> zs)
{
    GLuint zsbuffer = 0;
    GLenum zs_attachment = GL_DEPTH_STENCIL_ATTACHMENT;

    if (cur_zstencil_buffer.lock() == zs)
        return true;

    cur_zstencil_buffer = zs;

    if (zs) {
        zsbuffer = zs->buffer;
        zs_attachment = zs->attachment;
    }

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, zs_attachment, GL_RENDERBUFFER, zsbuffer);
    if (!gl_success("glFramebufferRenderbuffer"))
        return false;

    return true;
}

fbo_info::~fbo_info() {
    glDeleteFramebuffers(1, &fbo);
    gl_success("glDeleteFramebuffers");
}

struct gs_texture_base {
    gs_color_format format{};
    GLenum gl_format{};
    GLenum gl_target{};
    GLenum gl_internal_format{};
    GLenum gl_type{};
    GLuint texture{};
    uint32_t levels{};
    bool is_dynamic{};
    bool is_render_target{};
    bool is_dummy{};
    bool gen_mipmaps{};

    std::shared_ptr<fbo_info> fbo{};
};

struct gs_texture_private
{
    struct gs_texture_base base{};

    uint32_t width{};
    uint32_t height{};
    bool gen_mipmaps{};
    GLuint unpack_buffer{};
    GLsizeiptr size{};
};

gs_texture::gs_texture()
{
    d_ptr = std::make_unique<gs_texture_private>();
}

gs_texture::~gs_texture()
{
    if (!d_ptr->base.is_dummy && d_ptr->base.is_dynamic && d_ptr->unpack_buffer)
        gl_delete_buffers(1, &d_ptr->unpack_buffer);

    if (d_ptr->base.texture)
        gl_delete_textures(1, &d_ptr->base.texture);
}

bool gs_texture::create(uint32_t width, uint32_t height, gs_color_format color_format, uint32_t levels, const uint8_t **data, uint32_t flags)
{
    d_ptr->base.format = color_format;
    d_ptr->base.levels = levels;
    d_ptr->base.gl_format = convert_gs_format(color_format);
    d_ptr->base.gl_internal_format = convert_gs_internal_format(color_format);
    d_ptr->base.gl_type = get_gl_format_type(color_format);
    d_ptr->base.gl_target = GL_TEXTURE_2D;
    d_ptr->base.is_dynamic = (flags & GS_DYNAMIC) != 0;
    d_ptr->base.is_render_target = (flags & GS_RENDER_TARGET) != 0;
    d_ptr->base.is_dummy = (flags & GS_GL_DUMMYTEX) != 0;
    d_ptr->base.gen_mipmaps = (flags & GS_BUILD_MIPMAPS) != 0;
    d_ptr->width = width;
    d_ptr->height = height;

    if (!gl_gen_textures(1, &d_ptr->base.texture))
        return false;

    if (!d_ptr->base.is_dummy) {
        if (d_ptr->base.is_dynamic && !create_pixel_unpack_buffer())
            return false;
        if (!upload_texture_2d(data))
            return false;
    } else {
        if (!gl_bind_texture(GL_TEXTURE_2D, d_ptr->base.texture))
            return false;

        uint32_t row_size = d_ptr->width * gs_get_format_bpp(d_ptr->base.format);
        uint32_t tex_size = d_ptr->height * row_size / 8;
        bool compressed = gs_is_compressed_format(d_ptr->base.format);
        bool did_init = gl_init_face(GL_TEXTURE_2D, d_ptr->base.gl_type,
                                     1, d_ptr->base.gl_format,
                                     d_ptr->base.gl_internal_format,
                                     compressed, d_ptr->width,
                                     d_ptr->height, tex_size, NULL);
        did_init =
                gl_tex_param_i(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

        bool did_unbind = gl_bind_texture(GL_TEXTURE_2D, 0);
        if (!did_init || !did_unbind)
            return false;
    }

    return true;
}

uint32_t gs_texture::gs_texture_get_width()
{
    return d_ptr->width;
}

uint32_t gs_texture::gs_texture_get_height()
{
    return d_ptr->height;
}

gs_color_format gs_texture::gs_texture_get_color_format()
{
    return d_ptr->base.format;
}

bool gs_texture::gs_texture_map(uint8_t **ptr, uint32_t *linesize)
{
    if (!d_ptr->base.is_dynamic) {
        blog(LOG_ERROR, "Texture is not dynamic");
        goto fail;
    }

    if (!gl_bind_buffer(GL_PIXEL_UNPACK_BUFFER, d_ptr->unpack_buffer))
        goto fail;

#if defined __ANDROID__
    *ptr = (uint8_t *)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, d_ptr->size, GL_MAP_WRITE_BIT);
#elif defined WIN32
    *ptr = (uint8_t *)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
#endif
    if (!gl_success("glMapBuffer"))
        goto fail;

    gl_bind_buffer(GL_PIXEL_UNPACK_BUFFER, 0);

    *linesize = d_ptr->width * gs_get_format_bpp(d_ptr->base.format) / 8;
    *linesize = (*linesize + 3) & 0xFFFFFFFC;
    return true;

fail:
    blog(LOG_ERROR, "gs_texture_map (GL) failed");
    return false;
}

void gs_texture::gs_texture_unmap()
{
    if (!gl_bind_buffer(GL_PIXEL_UNPACK_BUFFER, d_ptr->unpack_buffer))
        goto failed;

    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    if (!gl_success("glUnmapBuffer"))
        goto failed;

    if (!gl_bind_texture(GL_TEXTURE_2D, d_ptr->base.texture))
        goto failed;

    glTexImage2D(GL_TEXTURE_2D, 0, d_ptr->base.gl_internal_format, d_ptr->width,
                 d_ptr->height, 0, d_ptr->base.gl_format, d_ptr->base.gl_type, 0);
    if (!gl_success("glTexImage2D"))
        goto failed;

    gl_bind_buffer(GL_PIXEL_UNPACK_BUFFER, 0);
    gl_bind_texture(GL_TEXTURE_2D, 0);
    return;

failed:
    gl_bind_buffer(GL_PIXEL_UNPACK_BUFFER, 0);
    gl_bind_texture(GL_TEXTURE_2D, 0);
    blog(LOG_ERROR, "gs_texture_unmap (GL) failed");
}

bool gs_texture::gs_texture_is_rect()
{
    return false;
}

bool gs_texture::gs_texture_is_render_target()
{
    return d_ptr->base.is_render_target;
}

std::shared_ptr<fbo_info> gs_texture::get_fbo()
{
    uint32_t width, height;
    if (!get_tex_dimensions( &width, &height))
        return nullptr;

    if (d_ptr->base.fbo && d_ptr->base.fbo->width == width &&
            d_ptr->base.fbo->height == height && d_ptr->base.fbo->format == d_ptr->base.format)
        return d_ptr->base.fbo;

    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    if (!gl_success("glGenFramebuffers"))
        return nullptr;

    d_ptr->base.fbo = std::make_shared<fbo_info>();

    d_ptr->base.fbo->fbo = fbo;
    d_ptr->base.fbo->width = width;
    d_ptr->base.fbo->height = height;
    d_ptr->base.fbo->format = d_ptr->base.format;
    d_ptr->base.fbo->cur_render_target.reset();
    d_ptr->base.fbo->cur_zstencil_buffer.reset();

    return d_ptr->base.fbo;
}

GLuint gs_texture::gs_texture_obj()
{
    return d_ptr->base.texture;
}

bool gs_texture::upload_texture_2d(const uint8_t **data)
{
    uint32_t row_size = d_ptr->width * gs_get_format_bpp(d_ptr->base.format);
    uint32_t tex_size = d_ptr->height * row_size / 8;
    uint32_t num_levels = d_ptr->base.levels;
    bool compressed = gs_is_compressed_format(d_ptr->base.format);
    bool success;

    if (!num_levels)
        num_levels = gs_get_total_levels(d_ptr->width, d_ptr->height);

    if (!gl_bind_texture(GL_TEXTURE_2D, d_ptr->base.texture))
        return false;

    success = gl_init_face(GL_TEXTURE_2D, d_ptr->base.gl_type, num_levels,
                           d_ptr->base.gl_format,
                           d_ptr->base.gl_internal_format, compressed,
                           d_ptr->width, d_ptr->height, tex_size, &data);

    if (!gl_tex_param_i(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL,
                        num_levels - 1))
        success = false;
    if (!gl_bind_texture(GL_TEXTURE_2D, 0))
        success = false;

    return success;
}

bool gs_texture::get_tex_dimensions(uint32_t *width, uint32_t *height)
{
    *width = d_ptr->width;
    *height = d_ptr->height;
    return true;
}

bool gs_texture::create_pixel_unpack_buffer()
{
    GLsizeiptr size;
    bool success = true;

    if (!gl_gen_buffers(1, &d_ptr->unpack_buffer))
        return false;

    if (!gl_bind_buffer(GL_PIXEL_UNPACK_BUFFER, d_ptr->unpack_buffer))
        return false;

    size = d_ptr->width * gs_get_format_bpp(d_ptr->base.format);
    if (!gs_is_compressed_format(d_ptr->base.format)) {
        size /= 8;
        size = (size + 3) & 0xFFFFFFFC;
        size *= d_ptr->height;
    } else {
        size *= d_ptr->height;
        size /= 8;
    }

    glBufferData(GL_PIXEL_UNPACK_BUFFER, size, 0, GL_DYNAMIC_DRAW);
    if (!gl_success("glBufferData"))
        success = false;

    if (!gl_bind_buffer(GL_PIXEL_UNPACK_BUFFER, 0))
        success = false;

    d_ptr->size = size;
    return success;
}

static inline bool is_pow2(uint32_t size)
{
    return size >= 2 && (size & (size - 1)) == 0;
}

std::shared_ptr<gs_texture> gs_texture_create(uint32_t width, uint32_t height, gs_color_format color_format, uint32_t levels, const uint8_t **data, uint32_t flags)
{
    bool pow2tex = is_pow2(width) && is_pow2(height);
    bool uses_mipmaps = (flags & GS_BUILD_MIPMAPS || levels != 1);

    if (!gs_valid("gs_texture_create"))
        return NULL;

    if (uses_mipmaps && !pow2tex) {
        blog(LOG_WARNING, "Cannot use mipmaps with a "
                          "non-power-of-two texture.  Disabling "
                          "mipmaps for this texture.");

        uses_mipmaps = false;
        flags &= ~GS_BUILD_MIPMAPS;
        levels = 1;
    }

    if (uses_mipmaps && flags & GS_RENDER_TARGET) {
        blog(LOG_WARNING, "Cannot use mipmaps with render targets.  "
                          "Disabling mipmaps for this texture.");
        flags &= ~GS_BUILD_MIPMAPS;
        levels = 1;
    }

    auto tex = std::make_shared<gs_texture>();
    if (!tex->create(width, height, color_format, levels, data, flags)) {
        blog(LOG_DEBUG, "Cannot create texture, width: %d, height:%d, format: %d", width, height, (int)color_format);
        return nullptr;
    }

    blog(LOG_DEBUG, "gs_texture_create: success.");

    return tex;
}

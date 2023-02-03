#pragma once

#include "gs_subsystem_info.h"
#include "gl-helpers.h"
#include <memory>

struct gs_zstencil_buffer {
    GLuint buffer{};
    GLuint attachment{};
    GLenum format{};
};

class gs_texture;
struct fbo_info {
    GLuint fbo{};
    uint32_t width{};
    uint32_t height{};
    gs_color_format format{};

    std::weak_ptr<gs_texture> cur_render_target{};
    std::weak_ptr<gs_zstencil_buffer> cur_zstencil_buffer{};

    bool attach_rendertarget(std::shared_ptr<gs_texture> tex);
    bool attach_zstencil(std::shared_ptr<gs_zstencil_buffer> zs);

    ~fbo_info();
};

struct gs_zstencil_buffer;
struct gs_texture_private;
class gs_texture
{
public:
    gs_texture();
    ~gs_texture();

    bool create(uint32_t width, uint32_t height, gs_color_format color_format, uint32_t levels, const uint8_t **data, uint32_t flags);

    uint32_t gs_texture_get_width();
    uint32_t gs_texture_get_height();

    gs_color_format gs_texture_get_color_format();

    bool gs_texture_map(uint8_t **ptr, uint32_t *linesize);
    void gs_texture_unmap();

    bool gs_texture_is_rect();
    bool gs_texture_is_render_target();

    std::shared_ptr<fbo_info> get_fbo();
    GLuint gs_texture_obj();

private:
    bool create_pixel_unpack_buffer();
    bool upload_texture_2d(const uint8_t **data);
    bool gs_set_target(int side, std::shared_ptr<gs_zstencil_buffer> zs);
    bool get_tex_dimensions(uint32_t *width, uint32_t *height);

private:
    std::unique_ptr<gs_texture_private> d_ptr{};
};

std::shared_ptr<gs_texture> gs_texture_create(uint32_t width, uint32_t height, gs_color_format color_format, uint32_t levels, const uint8_t **data, uint32_t flags);


#pragma once

#if defined __ANDROID__
#include <GLES3/gl3.h>
#include <EGL/egl.h>
#elif defined WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <glad/glad_wgl.h>
#endif

#define GS_SUCCESS 0
#define GS_ERROR_FAIL -1
#define GS_ERROR_MODULE_NOT_FOUND -2
#define GS_ERROR_NOT_SUPPORTED -3

#define GS_BUILD_MIPMAPS (1 << 0)
#define GS_DYNAMIC (1 << 1)
#define GS_RENDER_TARGET (1 << 2)
#define GS_GL_DUMMYTEX (1 << 3) /**<< texture with no allocated texture data */
#define GS_DUP_BUFFER \
    (1 << 4) /**<< do not pass buffer ownership when
                 *    creating a vertex/index buffer */

enum class gs_blend_type {
    GS_BLEND_ZERO,
    GS_BLEND_ONE,
    GS_BLEND_SRCCOLOR,
    GS_BLEND_INVSRCCOLOR,
    GS_BLEND_SRCALPHA,
    GS_BLEND_INVSRCALPHA,
    GS_BLEND_DSTCOLOR,
    GS_BLEND_INVDSTCOLOR,
    GS_BLEND_DSTALPHA,
    GS_BLEND_INVDSTALPHA,
    GS_BLEND_SRCALPHASAT,
};

static inline GLenum convert_gs_blend_type(gs_blend_type type)
{
    switch (type) {
    case gs_blend_type::GS_BLEND_ZERO:
        return GL_ZERO;
    case gs_blend_type::GS_BLEND_ONE:
        return GL_ONE;
    case gs_blend_type::GS_BLEND_SRCCOLOR:
        return GL_SRC_COLOR;
    case gs_blend_type::GS_BLEND_INVSRCCOLOR:
        return GL_ONE_MINUS_SRC_COLOR;
    case gs_blend_type::GS_BLEND_SRCALPHA:
        return GL_SRC_ALPHA;
    case gs_blend_type::GS_BLEND_INVSRCALPHA:
        return GL_ONE_MINUS_SRC_ALPHA;
    case gs_blend_type::GS_BLEND_DSTCOLOR:
        return GL_DST_COLOR;
    case gs_blend_type::GS_BLEND_INVDSTCOLOR:
        return GL_ONE_MINUS_DST_COLOR;
    case gs_blend_type::GS_BLEND_DSTALPHA:
        return GL_DST_ALPHA;
    case gs_blend_type::GS_BLEND_INVDSTALPHA:
        return GL_ONE_MINUS_DST_ALPHA;
    case gs_blend_type::GS_BLEND_SRCALPHASAT:
        return GL_SRC_ALPHA_SATURATE;
    }

    return GL_ONE;
}

enum class gs_color_format {
    GS_UNKNOWN,
    GS_A8,
    GS_R8,
    GS_RGBA,
    GS_RGBA16F,
    GS_RGBA32F,
    GS_RG16F,
    GS_RG32F,
    GS_R16F,
    GS_R32F,
    GS_R8G8,
};

static inline GLenum convert_gs_format(gs_color_format format)
{
    switch (format) {
    case gs_color_format::GS_A8:
        return GL_RED;
    case gs_color_format::GS_R8:
        return GL_RED;
    case gs_color_format::GS_RGBA:
        return GL_RGBA;
    case gs_color_format::GS_RGBA16F:
        return GL_RGBA;
    case gs_color_format::GS_RGBA32F:
        return GL_RGBA;
    case gs_color_format::GS_RG16F:
        return GL_RG;
    case gs_color_format::GS_RG32F:
        return GL_RG;
    case gs_color_format::GS_R8G8:
        return GL_RG;
    case gs_color_format::GS_R16F:
        return GL_RED;
    case gs_color_format::GS_R32F:
        return GL_RED;
    case gs_color_format::GS_UNKNOWN:
        return 0;
    }

    return 0;
}

static inline GLenum convert_gs_internal_format(gs_color_format format)
{
    switch (format) {
    case gs_color_format::GS_A8:
        return GL_R8; /* NOTE: use GL_TEXTURE_SWIZZLE_x */
    case gs_color_format::GS_R8:
        return GL_R8;
    case gs_color_format::GS_RGBA:
        return GL_RGBA;
    case gs_color_format::GS_RGBA16F:
        return GL_RGBA16F;
    case gs_color_format::GS_RGBA32F:
        return GL_RGBA32F;
    case gs_color_format::GS_RG16F:
        return GL_RG16F;
    case gs_color_format::GS_RG32F:
        return GL_RG32F;
    case gs_color_format::GS_R8G8:
        return GL_RG8;
    case gs_color_format::GS_R16F:
        return GL_R16F;
    case gs_color_format::GS_R32F:
        return GL_R32F;
    case gs_color_format::GS_UNKNOWN:
        return 0;
    }

    return 0;
}

static inline GLenum get_gl_format_type(gs_color_format format)
{
    switch (format) {
    case gs_color_format::GS_A8:
        return GL_UNSIGNED_BYTE;
    case gs_color_format::GS_R8:
        return GL_UNSIGNED_BYTE;
    case gs_color_format::GS_RGBA:
        return GL_UNSIGNED_BYTE;
    case gs_color_format::GS_RGBA16F:
        return GL_UNSIGNED_SHORT;
    case gs_color_format::GS_RGBA32F:
        return GL_FLOAT;
    case gs_color_format::GS_RG16F:
        return GL_UNSIGNED_SHORT;
    case gs_color_format::GS_RG32F:
        return GL_FLOAT;
    case gs_color_format::GS_R8G8:
        return GL_UNSIGNED_BYTE;
    case gs_color_format::GS_R16F:
        return GL_UNSIGNED_SHORT;
    case gs_color_format::GS_R32F:
        return GL_FLOAT;
    case gs_color_format::GS_UNKNOWN:
        return 0;
    }

    return GL_UNSIGNED_BYTE;
}

enum class gs_zstencil_format {
    GS_ZS_NONE,
    GS_Z16,
    GS_Z24_S8,
    GS_Z32F,
    GS_Z32F_S8X24,
};

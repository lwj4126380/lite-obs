#pragma once

#ifdef __ANDROID__
#include <GLES3/gl3.h>
#include <EGL/egl.h>
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

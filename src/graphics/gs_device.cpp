#include "gs_device.h"
#include "graphics.h"
#include "util/log.h"

#include "gl-helpers.h"
#include "gs_vertexbuffer.h"

struct gl_platform
{
    EGLDisplay display{};
    EGLSurface surface{};
    EGLContext context{};

    ~gl_platform() {
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroyContext(display, context);
        eglDestroySurface(display, surface);
        eglTerminate(display);
    }
};

struct gs_device_private
{
    std::unique_ptr<gl_platform> plat{};

    GLuint empty_vao{};
};

gs_device::gs_device()
{
    d_ptr = std::make_unique<gs_device_private>();
}

gs_device::~gs_device()
{

}

int gs_device::device_create()
{
    blog(LOG_INFO, "---------------------------------");
    blog(LOG_INFO, "Initializing OpenGL...");

    d_ptr->plat = gl_platform_create();
    if (!d_ptr->plat)
        return GS_ERROR_FAIL;

    const char *glVendor = (const char *)glGetString(GL_VENDOR);
    const char *glRenderer = (const char *)glGetString(GL_RENDERER);

    blog(LOG_INFO, "Loading up OpenGL on adapter %s %s", glVendor, glRenderer);

    const char *glVersion = (const char *)glGetString(GL_VERSION);
    const char *glShadingLanguage =
            (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);

    blog(LOG_INFO, "OpenGL loaded successfully, version %s, shading " "language %s", glVersion, glShadingLanguage);

    gl_enable(GL_CULL_FACE);
    gl_gen_vertex_arrays(1, &d_ptr->empty_vao);

    device_leave_context();

    return GS_SUCCESS;
}

void gs_device::device_enter_context()
{
    if (!d_ptr->plat)
        return;

    if (!eglMakeCurrent(d_ptr->plat->display, d_ptr->plat->surface, d_ptr->plat->surface, d_ptr->plat->context)) {
        blog(LOG_DEBUG, "eglMakeCurrent() returned error %d", eglGetError());
    }
}

void gs_device::device_leave_context()
{
    if (!d_ptr->plat)
        return;

    eglMakeCurrent(d_ptr->plat->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
}

std::unique_ptr<gs_vertexbuffer> gs_device::device_vertexbuffer_create(std::unique_ptr<gs_vb_data> data, uint32_t flags)
{
    auto vb = std::make_unique<gs_vertexbuffer>(std::move(data), flags);
    if (!vb->create_buffers())
        return nullptr;

    return vb;
}

void gs_device::device_blend_function_separate(gs_blend_type src_c, gs_blend_type dest_c, gs_blend_type src_a, gs_blend_type dest_a)
{
    GLenum gl_src_c = convert_gs_blend_type(src_c);
    GLenum gl_dst_c = convert_gs_blend_type(dest_c);
    GLenum gl_src_a = convert_gs_blend_type(src_a);
    GLenum gl_dst_a = convert_gs_blend_type(dest_a);

    glBlendFuncSeparate(gl_src_c, gl_dst_c, gl_src_a, gl_dst_a);
    if (!gl_success("glBlendFuncSeparate"))
        blog(LOG_ERROR, "device_blend_function_separate (GL) failed");
}

std::unique_ptr<gl_platform> gs_device::gl_platform_create()
{
    const EGLint attribs[] = {
        EGL_RENDERABLE_TYPE,
        EGL_OPENGL_ES3_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 0,
        EGL_STENCIL_SIZE, 0,
        EGL_NONE
    };
    EGLDisplay display;
    EGLConfig config;
    EGLint numConfigs;
    EGLSurface surface;
    EGLContext context;

    blog(LOG_DEBUG, "Initializing context");

    if ((display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY) {
        blog(LOG_DEBUG, "eglGetDisplay() returned error %d", eglGetError());
        return nullptr;
    }
    EGLint major = 0, minor = 0;
    if (!eglInitialize(display, &major, &minor)) {
        blog(LOG_DEBUG, "eglInitialize() returned error %d", eglGetError());
        return nullptr;
    }

    eglBindAPI(EGL_OPENGL_ES_API);

    if (!eglChooseConfig(display, attribs, &config, 1, &numConfigs)) {
        blog(LOG_DEBUG, "eglChooseConfig() returned error %d", eglGetError());

        return nullptr;
    }

    if (!(surface = eglCreatePbufferSurface(display, config, nullptr))) {
        blog(LOG_DEBUG, "eglCreateWindowSurface() returned error %d", eglGetError());
        return nullptr;
    }

    const EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,  //Request opengl ES2.0
        EGL_NONE
    };
    if (!(context = eglCreateContext(display, config, nullptr, contextAttribs))) {
        blog(LOG_DEBUG, "eglCreateContext() returned error %d", eglGetError());

        return nullptr;
    }

    if (!eglMakeCurrent(display, surface, surface, context)) {
        blog(LOG_DEBUG, "eglMakeCurrent() returned error %d", eglGetError());
        return nullptr;
    }

    blog(LOG_DEBUG, "egl create opengles context success");

    auto plat = std::make_unique<gl_platform>();
    plat->context = context;
    plat->display = display;
    plat->surface = surface;

    return std::move(plat);
}

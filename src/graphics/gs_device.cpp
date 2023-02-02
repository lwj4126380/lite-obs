#include "gs_device.h"
#include "graphics.h"
#include "util/log.h"

#include "gl-helpers.h"
#include "gs_vertexbuffer.h"

#if defined __ANDROID__
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
#elif defined WIN32
/* Basically swapchain-specific information.  Fortunately for windows this is
 * super basic stuff */
struct gl_windowinfo {
    HWND hwnd;
    HDC hdc;
};

/* Like the other subsystems, the GL subsystem has one swap chain created by
 * default. */
struct gl_platform {
    HGLRC hrc;
    struct gl_windowinfo window;

    ~gl_platform() {
        if (hrc) {
            wglMakeCurrent(NULL, NULL);
            wglDeleteContext(hrc);
        }

        if (window.hdc)
            ReleaseDC(window.hwnd, window.hdc);
        if (window.hwnd)
            DestroyWindow(window.hwnd);
    }
};
#endif

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

#if defined __ANDROID
    if (!eglMakeCurrent(d_ptr->plat->display, d_ptr->plat->surface, d_ptr->plat->surface, d_ptr->plat->context)) {
        blog(LOG_DEBUG, "eglMakeCurrent() returned error %d", eglGetError());
    }
#elif defined WIN32
    HDC hdc = d_ptr->plat->window.hdc;
    if (!wglMakeCurrent(hdc, d_ptr->plat->hrc))
        blog(LOG_ERROR, "device_enter_context (GL) failed");
#endif
}

void gs_device::device_leave_context()
{
    if (!d_ptr->plat)
        return;

#if defined __ANDROID
    eglMakeCurrent(d_ptr->plat->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
#elif defined WIN32
    wglMakeCurrent(NULL, NULL);
#endif
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

#if defined __ANDROID__
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
#elif defined WIN32
struct dummy_context {
    HWND hwnd{};
    HGLRC hrc{};
    HDC hdc{};

    ~dummy_context() {
        wglMakeCurrent(NULL, NULL);
        if (hrc)
            wglDeleteContext(hrc);

        if (hwnd)
            DestroyWindow(hwnd);
    }
};

struct gs_init_data {
    void *hwnd{};
    uint32_t cx{}, cy{};
    uint32_t num_backbuffers{};
    gs_color_format format{};
    gs_zstencil_format zsformat{};
    uint32_t adapter{};
};

static const char *dummy_window_class = "GLDummyWindow";
static bool registered_dummy_window_class = false;
/* Need a dummy window for the dummy context */
static bool gl_register_dummy_window_class(void)
{
    WNDCLASSA wc;
    if (registered_dummy_window_class)
        return true;

    memset(&wc, 0, sizeof(wc));
    wc.style = CS_OWNDC;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpfnWndProc = DefWindowProc;
    wc.lpszClassName = dummy_window_class;

    if (!RegisterClassA(&wc)) {
        blog(LOG_ERROR, "Could not create dummy window class");
        return false;
    }

    registered_dummy_window_class = true;
    return true;
}

static inline HWND gl_create_dummy_window(void)
{
    HWND hwnd = CreateWindowExA(0, dummy_window_class, "Dummy GL Window",
                    WS_POPUP, 0, 0, 2, 2, NULL, NULL,
                    GetModuleHandle(NULL), NULL);
    if (!hwnd)
        blog(LOG_ERROR, "Could not create dummy context window");

    return hwnd;
}

/* would use designated initializers but Microsoft sort of sucks */
static inline void init_dummy_pixel_format(PIXELFORMATDESCRIPTOR *pfd)
{
    memset(pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
    pfd->nSize = sizeof(pfd);
    pfd->nVersion = 1;
    pfd->iPixelType = PFD_TYPE_RGBA;
    pfd->cColorBits = 32;
    pfd->cDepthBits = 24;
    pfd->cStencilBits = 8;
    pfd->iLayerType = PFD_MAIN_PLANE;
    pfd->dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |
               PFD_DOUBLEBUFFER;
}

static inline HGLRC gl_init_basic_context(HDC hdc)
{
    HGLRC hglrc = wglCreateContext(hdc);
    if (!hglrc) {
        blog(LOG_ERROR, "wglCreateContext failed, %lu", GetLastError());
        return NULL;
    }

    if (!wglMakeCurrent(hdc, hglrc)) {
        wglDeleteContext(hglrc);
        return NULL;
    }

    return hglrc;
}

static bool gl_dummy_context_init(dummy_context *dummy)
{
    PIXELFORMATDESCRIPTOR pfd;
    int format_index;

    if (!gl_register_dummy_window_class())
        return false;

    dummy->hwnd = gl_create_dummy_window();
    if (!dummy->hwnd)
        return false;

    dummy->hdc = GetDC(dummy->hwnd);

    init_dummy_pixel_format(&pfd);
    format_index = ChoosePixelFormat(dummy->hdc, &pfd);
    if (!format_index) {
        blog(LOG_ERROR, "Dummy ChoosePixelFormat failed, %lu",
             GetLastError());
        return false;
    }

    if (!SetPixelFormat(dummy->hdc, format_index, &pfd)) {
        blog(LOG_ERROR, "Dummy SetPixelFormat failed, %lu",
             GetLastError());
        return false;
    }

    dummy->hrc = gl_init_basic_context(dummy->hdc);
    if (!dummy->hrc) {
        blog(LOG_ERROR, "Failed to initialize dummy context");
        return false;
    }

    return true;
}

static inline void required_extension_error(const char *extension)
{
    blog(LOG_ERROR, "OpenGL extension %s is required", extension);
}
static bool gl_init_extensions(HDC hdc)
{
    if (!gladLoadWGL(hdc)) {
        blog(LOG_ERROR, "Failed to load WGL entry functions.");
        return false;
    }

    if (!GLAD_WGL_ARB_pixel_format) {
        required_extension_error("ARB_pixel_format");
        return false;
    }

    if (!GLAD_WGL_ARB_create_context) {
        required_extension_error("ARB_create_context");
        return false;
    }

    if (!GLAD_WGL_ARB_create_context_profile) {
        required_extension_error("ARB_create_context_profile");
        return false;
    }

    return true;
}

#define DUMMY_WNDCLASS "Dummy GL Window Class"

static bool register_dummy_class(void)
{
    static bool created = false;

    WNDCLASSA wc = {0};
    wc.style = CS_OWNDC;
    wc.hInstance = GetModuleHandleW(NULL);
    wc.lpfnWndProc = (WNDPROC)DefWindowProcA;
    wc.lpszClassName = DUMMY_WNDCLASS;

    if (created)
        return true;

    if (!RegisterClassA(&wc)) {
        blog(LOG_ERROR, "Failed to register dummy GL window class, %lu",
             GetLastError());
        return false;
    }

    created = true;
    return true;
}

static bool create_dummy_window(gl_platform *plat)
{
    plat->window.hwnd = CreateWindowExA(0, DUMMY_WNDCLASS,
                        "OpenGL Dummy Window", WS_POPUP, 0,
                        0, 1, 1, NULL, NULL,
                        GetModuleHandleW(NULL), NULL);
    if (!plat->window.hwnd) {
        blog(LOG_ERROR, "Failed to create dummy GL window, %lu",
             GetLastError());
        return false;
    }

    plat->window.hdc = GetDC(plat->window.hwnd);
    if (!plat->window.hdc) {
        blog(LOG_ERROR, "Failed to get dummy GL window DC (%lu)",
             GetLastError());
        return false;
    }

    return true;
}

/* For now, only support basic 32bit formats for graphics output. */
static inline int get_color_format_bits(gs_color_format format)
{
    switch (format) {
    case gs_color_format::GS_RGBA:
        return 32;
    default:
        return 0;
    }
}

static inline int get_depth_format_bits(gs_zstencil_format zsformat)
{
    switch (zsformat) {
    case gs_zstencil_format::GS_Z16:
        return 16;
    case gs_zstencil_format::GS_Z24_S8:
        return 24;
    default:
        return 0;
    }
}

static inline int get_stencil_format_bits(gs_zstencil_format zsformat)
{
    switch (zsformat) {
    case gs_zstencil_format::GS_Z24_S8:
        return 8;
    default:
        return 0;
    }
}

/* Creates the real pixel format for the target window */
static int gl_choose_pixel_format(HDC hdc, const gs_init_data *info)
{
    int color_bits = get_color_format_bits(info->format);
    int depth_bits = get_depth_format_bits(info->zsformat);
    int stencil_bits = get_stencil_format_bits(info->zsformat);
    UINT num_formats;
    BOOL success;
    int format;

    if (!color_bits) {
        blog(LOG_ERROR, "gl_init_pixel_format: color format not "
                "supported");
        return false;
    }

    int attribs[] = {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB, color_bits,
        WGL_DEPTH_BITS_ARB, depth_bits,
        WGL_STENCIL_BITS_ARB, stencil_bits,
        0, 0,
    };

    success = wglChoosePixelFormatARB(hdc, attribs, NULL, 1, &format,
                      &num_formats);
    if (!success || !num_formats) {
        blog(LOG_ERROR, "wglChoosePixelFormatARB failed, %lu",
             GetLastError());
        format = 0;
    }

    return format;
}

static inline bool gl_getpixelformat(HDC hdc, const gs_init_data *info, int *format, PIXELFORMATDESCRIPTOR *pfd)
{
    if (!format)
        return false;

    *format = gl_choose_pixel_format(hdc, info);

    if (!DescribePixelFormat(hdc, *format, sizeof(*pfd), pfd)) {
        blog(LOG_ERROR, "DescribePixelFormat failed, %lu",
             GetLastError());
        return false;
    }

    return true;
}

static inline bool gl_setpixelformat(HDC hdc, int format, PIXELFORMATDESCRIPTOR *pfd)
{
    if (!SetPixelFormat(hdc, format, pfd)) {
        blog(LOG_ERROR, "SetPixelFormat failed, %lu", GetLastError());
        return false;
    }

    return true;
}
static bool init_default_swap(gl_platform *plat, int pixel_format, PIXELFORMATDESCRIPTOR *pfd)
{
    if (!gl_setpixelformat(plat->window.hdc, pixel_format, pfd))
        return false;

    return true;
}

std::unique_ptr<gl_platform> gs_device::gl_platform_create()
{
    auto dummy = std::make_unique<dummy_context>();
    gs_init_data info;
    info.format = gs_color_format::GS_RGBA;
    info.zsformat = gs_zstencil_format::GS_ZS_NONE;
    int pixel_format;
    PIXELFORMATDESCRIPTOR pfd;

    if (!gl_dummy_context_init(dummy.get()))
        return nullptr;

    if (!gl_init_extensions(dummy->hdc))
        return nullptr;

    if (!register_dummy_class())
            return nullptr;

    auto plat = std::make_unique<gl_platform>();
    if (!create_dummy_window(plat.get()))
        return nullptr;

    if (!gl_getpixelformat(dummy->hdc, &info, &pixel_format, &pfd))
        return nullptr;

    dummy.reset();

    if (!init_default_swap(plat.get(), pixel_format, &pfd))
        return nullptr;

    plat->hrc = gl_init_basic_context(plat->window.hdc);
    if (!plat->hrc)
        return nullptr;

    if (!gladLoadGL()) {
        blog(LOG_ERROR, "Failed to initialize OpenGL entry functions.");
        return nullptr;
    }

    return plat;
}
#endif

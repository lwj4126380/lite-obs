#include "gs_subsystem.h"
#include <mutex>
#include "gs_device.h"
#include "gs_vertexbuffer.h"
#include "gs_texture.h"
#include "gs_shader.h"
#include "shaders.h"
#include "gs_program.h"

#include "util/log.h"

#include <list>
#include <map>
#include <glm/mat4x4.hpp>

struct graphics_subsystem_private
{
    std::shared_ptr<gs_device> device{};

    std::list<glm::mat4x4> matrix_stack{};
    size_t cur_matrix{};

    glm::mat4x4 projection{0};

    std::shared_ptr<gs_vertexbuffer> sprite_buffer{};

    std::recursive_mutex mutex;
    std::mutex effect_mutex;

    std::map<std::string, std::shared_ptr<gs_program>> effects{};

    std::atomic_long ref{};
};

static thread_local graphics_subsystem *thread_graphics = NULL;

bool gs_valid(const char *f)
{
    if (!thread_graphics) {
        blog(LOG_DEBUG, "%s: called while not in a graphics context",
             f);
        return false;
    }

    return true;
}

graphics_subsystem::graphics_subsystem()
{
    d_ptr = std::make_unique<graphics_subsystem_private>();
}

graphics_subsystem::~graphics_subsystem()
{
    while (thread_graphics)
        gs_leave_context();

    if (d_ptr->device) {

        thread_graphics = this;
        d_ptr->device->device_enter_context();
        d_ptr->sprite_buffer.reset();
        d_ptr.reset();
        thread_graphics = nullptr;
    }
}

int graphics_subsystem::gs_create()
{
    auto device = std::make_shared<gs_device>();
    int errcode = device->device_create();
    if (errcode != GS_SUCCESS)
        return errcode;

    d_ptr->device = std::move(device);

    if (!graphics_init())
        return GS_ERROR_FAIL;

    return GS_SUCCESS;
}

int graphics_subsystem::gs_effect_init()
{
    for (int i = 0; i < all_shaders.size(); ++i) {
        const gs_shader_item &item = all_shaders[i];

        std::shared_ptr<gs_shader> vertex_shader{};
        std::shared_ptr<gs_shader> pixel_shader{};
        for (int j = 0; j < item.shaders.size(); ++j) {
            std::shared_ptr<gs_shader> shader = std::make_shared<gs_shader>();
            if (!shader->gs_shader_init(item.shaders.at(j))) {
                blog(LOG_DEBUG, "%s shader init fail.", item.name.c_str());
                continue;
            }
            if (shader->type() == gs_shader_type::GS_SHADER_VERTEX)
                vertex_shader = shader;
            else if (shader->type() == gs_shader_type::GS_SHADER_PIXEL)
                pixel_shader = shader;
        }

        if (vertex_shader && pixel_shader) {
            auto program = std::make_shared<gs_program>();
            if (program->gs_program_create(vertex_shader, pixel_shader)) {
                d_ptr->effects.insert({item.name, program});
            }
        }
    }

    return GS_SUCCESS;
}

std::shared_ptr<gs_program> graphics_subsystem::gs_get_effect_by_name(const char *name)
{
    if (d_ptr->effects.contains(name))
        return d_ptr->effects.at(name);

    return nullptr;
}

void graphics_subsystem::gs_draw_sprite(std::shared_ptr<gs_texture> tex, uint32_t flip, uint32_t width, uint32_t height)
{
    if (!tex && (!width || !height)) {
        blog(LOG_ERROR, "A sprite cannot be drawn without "
                        "a width/height");
        return;
    }

    float fcx = width ? (float)width : (float)tex->gs_texture_get_width();
    float fcy = height ? (float)height : (float)tex->gs_texture_get_height();

    auto data = d_ptr->sprite_buffer->gs_vertexbuffer_get_data();
    data->build_sprite_norm(fcx, fcy, flip);
    d_ptr->sprite_buffer->gs_vertexbuffer_flush();

    d_ptr->device->gs_device_load_vertexbuffer(d_ptr->sprite_buffer);
    d_ptr->device->gs_device_load_indexbuffer(nullptr);

    d_ptr->device->gs_device_draw(d_ptr->effects["default_effect"], gs_draw_mode::GS_TRISTRIP, 0, 0);
}

bool graphics_subsystem::graphics_init()
{
    glm::mat4x4 top_mat(1);
    d_ptr->matrix_stack.push_back(top_mat);

    gs_device_context_helper context_helper(d_ptr->device.get());

    if (!graphics_init_sprite_vb())
        return false;

    d_ptr->device->device_blend_function_separate(gs_blend_type::GS_BLEND_SRCALPHA,
                                                  gs_blend_type::GS_BLEND_INVSRCALPHA,
                                                  gs_blend_type::GS_BLEND_ONE,
                                                  gs_blend_type::GS_BLEND_INVSRCALPHA);

    return true;
}

bool graphics_subsystem::graphics_init_sprite_vb()
{
    auto vb = std::make_shared<gs_vertexbuffer>();
    if (!vb->gs_vertexbuffer_init_sprite())
        return false;

    d_ptr->sprite_buffer = std::move(vb);
    return true;
}


void gs_enter_contex(std::unique_ptr<graphics_subsystem> &graphics)
{
    bool is_current = thread_graphics == graphics.get();
    if (thread_graphics && !is_current) {
        while (thread_graphics)
            gs_leave_context();
    }

    if (!is_current) {
        graphics->d_ptr->mutex.lock();
        graphics->d_ptr->device->device_enter_context();
        thread_graphics = graphics.get();
    }

    graphics->d_ptr->ref++;
}

void gs_leave_context()
{
    if (gs_valid("gs_leave_context")) {
        if (!--thread_graphics->d_ptr->ref) {
            graphics_subsystem *graphics = thread_graphics;

            graphics->d_ptr->device->device_leave_context();
            graphics->d_ptr->mutex.unlock();
            thread_graphics = nullptr;
        }
    }
}

graphics_subsystem *gs_graphics_subsystem()
{
    return thread_graphics;
}

void gs_set_render_target(std::shared_ptr<gs_texture> tex, std::shared_ptr<gs_zstencil_buffer> zs)
{
    if (!gs_valid("gs_set_render_target"))
        return;

    if (!thread_graphics->d_ptr->device->gs_device_set_render_target(tex, zs))
        blog(LOG_ERROR, "device_set_render_target (GL) failed");
}

void gs_enable_depth_test(bool enable)
{
    if (!gs_valid("gs_enable_depth_test"))
        return;

    if (enable)
        gl_enable(GL_DEPTH_TEST);
    else
        gl_disable(GL_DEPTH_TEST);
}

void gs_set_cull_mode(gs_cull_mode mode)
{
    if (!gs_valid("gs_set_cull_mode"))
        return;

    thread_graphics->d_ptr->device->gs_device_set_cull_mode(mode);
}

void gs_ortho(float left, float right, float top, float bottom, float znear, float zfar)
{
    if (!gs_valid("gs_ortho"))
        return;

    thread_graphics->d_ptr->device->gs_device_ortho(left, right, top, bottom, znear, zfar);
}

void gs_set_viewport(int x, int y, int width, int height)
{
    if (!gs_valid("gs_set_view_port"))
        return;

    thread_graphics->d_ptr->device->gs_device_set_viewport(x, y, width, height);
}

void gs_clear(uint32_t clear_flags, glm::vec4 *color, float depth, uint8_t stencil)
{
    if (!gs_valid("gs_clear"))
        return;

    GLbitfield gl_flags = 0;

    if (clear_flags & GS_CLEAR_COLOR) {
        glClearColor(color->x, color->y, color->z, color->w);
        gl_flags |= GL_COLOR_BUFFER_BIT;
    }

    if (clear_flags & GS_CLEAR_DEPTH) {
#if defined __ANDROID__
        glClearDepthf(depth);
#else
        glClearDepth(depth);
#endif
        gl_flags |= GL_DEPTH_BUFFER_BIT;
    }

    if (clear_flags & GS_CLEAR_STENCIL) {
        glClearStencil(stencil);
        gl_flags |= GL_STENCIL_BUFFER_BIT;
    }

    glClear(gl_flags);
    if (!gl_success("glClear"))
        blog(LOG_ERROR, "device_clear (GL) failed");
}

void gs_set_render_size(uint32_t width, uint32_t height)
{
    if (!gs_valid("gs_set_render_size"))
        return;

    gs_enable_depth_test(false);
    gs_set_cull_mode(gs_cull_mode::GS_NEITHER);

    gs_ortho(0.0f, (float)width, 0.0f, (float)height, -100.0f, 100.0f);
    gs_set_viewport(0, 0, width, height);
}

void gs_load_texture(std::weak_ptr<gs_texture> tex, int unit)
{
    if (!gs_valid("gs_load_texture"))
        return;

    thread_graphics->d_ptr->device->gs_device_load_texture(tex, unit);
}

void gs_matrix_get(glm::mat4x4 &matrix)
{
    if (!gs_valid("gs_matrix_get"))
        return;

    matrix = thread_graphics->d_ptr->matrix_stack.back();
}

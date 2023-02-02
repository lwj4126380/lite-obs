#include "graphics.h"
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
    std::unique_ptr<gs_device> device{};

    std::list<glm::mat4x4> matrix_stack{};
    size_t cur_matrix{};

    glm::mat4x4 projection;

    std::unique_ptr<gs_vertexbuffer> sprite_buffer{};

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
    auto device = std::make_unique<gs_device>();
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
    auto vb = std::make_unique<gs_vertexbuffer>();
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

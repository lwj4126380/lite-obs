#include "graphics.h"
#include <mutex>
#include "gs_device.h"
#include "gs_vertexbuffer.h"

#include <list>
#include <glm/mat4x4.hpp>

struct graphics_subsystem_private
{
    std::unique_ptr<gs_device> device{};

    std::list<glm::mat4x4> matrix_stack;

    std::unique_ptr<gs_vertexbuffer> sprite_buffer;

    std::recursive_mutex mutex;
    std::mutex effect_mutex;
};

graphics_subsystem::graphics_subsystem()
{
    d_ptr = std::make_unique<graphics_subsystem_private>();
}

graphics_subsystem::~graphics_subsystem()
{

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
    auto vbd = std::make_unique<gs_vb_data>();
    vbd->num = 4;
    vbd->points.resize(4);
    vbd->num_tex = 1;
    vbd->tvarray.resize(1);
    vbd->tvarray[0].width = 2;
    vbd->tvarray[0].array = malloc(sizeof(glm::vec2) * 4);
    memset(vbd->tvarray[0].array, 0, sizeof(glm::vec2) * 4);

    d_ptr->sprite_buffer = d_ptr->device->device_vertexbuffer_create(std::move(vbd), GS_DYNAMIC);

    if (!d_ptr->sprite_buffer)
        return false;

    return true;
}

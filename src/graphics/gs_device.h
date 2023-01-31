#pragma once

#include <memory>
#include "graphics.h"

struct gs_device_private;
struct gl_platform;

class gs_vertexbuffer;
class gs_device
{
public:
    gs_device();
    ~gs_device();

    int device_create();

    void device_enter_context();
    void device_leave_context();

    std::unique_ptr<gs_vertexbuffer> device_vertexbuffer_create(std::unique_ptr<gs_vb_data> data, uint32_t flags);
    void device_blend_function_separate(gs_blend_type src_c, gs_blend_type dest_c, gs_blend_type src_a, gs_blend_type dest_a);

private:
    std::unique_ptr<gl_platform> gl_platform_create();

private:
    std::unique_ptr<gs_device_private> d_ptr{};
};

class gs_device_context_helper
{
public:
    gs_device_context_helper(gs_device *device) : d(device) {
        d->device_enter_context();
    }
    ~gs_device_context_helper() {
        d->device_leave_context();
    }

private:
    gs_device *d{};
};

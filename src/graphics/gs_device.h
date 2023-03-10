#pragma once

#include <memory>
#include "gs_subsystem.h"

struct gs_device_private;
struct gl_platform;

struct fbo_info;

class gs_vertexbuffer;
class gs_indexbuffer;
class gs_program;
class gs_device
{
public:
    gs_device();
    ~gs_device();

    static void gs_check_device_context();

    int device_create();
    void device_destroy();

    void device_enter_context();
    void device_leave_context();

    void device_blend_function_separate(gs_blend_type src_c, gs_blend_type dest_c, gs_blend_type src_a, gs_blend_type dest_a);

    bool gs_device_set_render_target(std::shared_ptr<gs_texture> tex, std::shared_ptr<gs_zstencil_buffer> zs);
    void gs_device_set_cull_mode(gs_cull_mode mode);

    void gs_device_ortho(float left, float right, float top, float bottom, float near, float far);
    void gs_device_set_viewport(int x, int y, int width, int height);
    void gs_device_get_viewport(gs_rect &rect);

    void gs_device_projection_push();
    void gs_device_projection_pop();

    std::shared_ptr<gs_texture> gs_device_get_render_target();
    std::shared_ptr<gs_zstencil_buffer> gs_device_get_zstencil_target();

    void gs_device_load_vertexbuffer(std::shared_ptr<gs_vertexbuffer> vb);
    void gs_device_load_indexbuffer(std::shared_ptr<gs_indexbuffer> ib);

    void gs_device_set_program(std::shared_ptr<gs_program> program);
    std::shared_ptr<gs_program> gs_device_program();
    void gs_device_draw(gs_draw_mode draw_mode, uint32_t start_vert, uint32_t num_verts);

    void gs_device_load_texture(std::weak_ptr<gs_texture> p_tex, int unit);

    void gs_device_clear_textures();
    void gs_device_load_default_pixelshader_samplers();

private:
    void *gl_platform_create();
    void gl_platform_destroy(void *plat);

    void device_enter_context_internal(void *param);
    void device_leave_context_internal(void *param);

    bool set_current_fbo(std::shared_ptr<fbo_info> fbo);
    uint32_t get_target_height();

    bool can_render(uint32_t num_verts);
    void update_viewproj_matrix();

private:
    std::unique_ptr<gs_device_private> d_ptr{};
};

std::shared_ptr<gs_device> gs_create_device();

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

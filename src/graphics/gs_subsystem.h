#pragma once

#include <memory>
#include <vector>
#include <string>

#include "gs_subsystem_info.h"
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

struct graphics_subsystem_private;
class gs_device;
class gs_texture;
class gs_program;
class gs_shader;
struct gs_zstencil_buffer;
class graphics_subsystem
{
public:
    graphics_subsystem();
    ~graphics_subsystem();

    int gs_create();
    int gs_effect_init();

    std::shared_ptr<gs_program> gs_get_effect_by_name(const char *name);
    void gs_draw_sprite(std::shared_ptr<gs_texture> tex, uint32_t flip, uint32_t width, uint32_t height);

private:
    bool graphics_init();
    bool graphics_init_sprite_vb();
    std::shared_ptr<gs_shader> shader_from_string(const std::string &shader_string, bool vertex_shader, std::string &out_name);

public:
    std::unique_ptr<graphics_subsystem_private> d_ptr{};
};

bool gs_valid(const char *f);
graphics_subsystem *gs_graphics_subsystem();

void gs_enter_contex(std::unique_ptr<graphics_subsystem> &graphics);
void gs_leave_context();

void gs_enable_depth_test(bool enable);
void gs_enable_blending(bool enable);
void gs_set_cull_mode(gs_cull_mode mode);
void gs_ortho(float left, float right, float top, float bottom, float znear, float zfar);
void gs_set_viewport(int x, int y, int width, int height);
void gs_get_viewport(gs_rect &rect);
void gs_clear(uint32_t clear_flags, glm::vec4 *color, float depth, uint8_t stencil);
void gs_flush();
void gs_set_render_size(uint32_t width, uint32_t height);
void gs_set_render_target(std::shared_ptr<gs_texture> tex, std::shared_ptr<gs_zstencil_buffer> zs);
void gs_set_cur_effect(std::shared_ptr<gs_program> program);
void gs_load_texture(std::weak_ptr<gs_texture> tex, int unit);

void gs_matrix_get(glm::mat4x4 &matrix);

void gs_viewport_push();
void gs_viewport_pop();
void gs_projection_push();
void gs_projection_pop();
void gs_matrix_push();
void gs_matrix_pop();
void gs_matrix_identity();

std::shared_ptr<gs_texture> gs_get_render_target();
std::shared_ptr<gs_zstencil_buffer> gs_get_zstencil_target();

void gs_draw(gs_draw_mode draw_mode, uint32_t start_vert, uint32_t num_verts);
void gs_technique_begin();
void gs_technique_end();

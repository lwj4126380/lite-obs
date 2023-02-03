#pragma once

#include <memory>
#include <vector>

#include "gs_subsystem_info.h"
#include <glm/vec4.hpp>

struct graphics_subsystem_private;
class gs_device;
class gs_texture;
class gs_program;
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

public:
    std::unique_ptr<graphics_subsystem_private> d_ptr{};
};

bool gs_valid(const char *f);
graphics_subsystem *gs_graphics_subsystem();

void gs_enter_contex(std::unique_ptr<graphics_subsystem> &graphics);
void gs_leave_context();

void gs_enable_depth_test(bool enable);
void gs_set_cull_mode(gs_cull_mode mode);
void gs_ortho(float left, float right, float top, float bottom, float znear, float zfar);
void gs_set_viewport(int x, int y, int width, int height);
void gs_clear(uint32_t clear_flags, glm::vec4 *color, float depth, uint8_t stencil);
void gs_set_render_size(uint32_t width, uint32_t height);
void gs_set_render_target(std::shared_ptr<gs_texture> tex, std::shared_ptr<gs_zstencil_buffer> zs);

void gs_load_texture(std::weak_ptr<gs_texture> tex, int unit);

static inline uint32_t gs_get_format_bpp(gs_color_format format)
{
    switch (format) {
    case gs_color_format::GS_A8:
        return 8;
    case gs_color_format::GS_R8:
        return 8;
    case gs_color_format::GS_RGBA:
        return 32;
    case gs_color_format::GS_RGBA16F:
        return 64;
    case gs_color_format::GS_RGBA32F:
        return 128;
    case gs_color_format::GS_RG16F:
        return 32;
    case gs_color_format::GS_RG32F:
        return 64;
    case gs_color_format::GS_R16F:
        return 16;
    case gs_color_format::GS_R32F:
        return 32;
    case gs_color_format::GS_R8G8:
        return 16;
    case gs_color_format::GS_UNKNOWN:
        return 0;
    }

    return 0;
}

static inline bool gs_is_compressed_format(gs_color_format format)
{
    return false;
}

static inline uint32_t gs_get_total_levels(uint32_t width, uint32_t height)
{
    uint32_t size = width > height ? width : height;
    uint32_t num_levels = 0;

    while (size > 1) {
        size /= 2;
        num_levels++;
    }

    return num_levels;
}

#pragma once

#include "gs_subsystem_info.h"
#include <memory>

struct gs_zstencil_buffer;
struct gs_texture_private;
class gs_texture
{
public:
    gs_texture();
    ~gs_texture();

    bool create(uint32_t width, uint32_t height, gs_color_format color_format, uint32_t levels, const uint8_t **data, uint32_t flags);

    uint32_t gs_texture_get_width();
    uint32_t gs_texture_get_height();

    gs_color_format gs_texture_get_color_format();

    bool gs_texture_map(uint8_t **ptr, uint32_t *linesize);
    void gs_texture_unmap();

    bool gs_texture_is_rect();

    bool gs_texture_is_render_target();
    bool gs_set_target(int side, std::shared_ptr<gs_zstencil_buffer> zs);

private:
    bool create_pixel_unpack_buffer();
    bool upload_texture_2d(const uint8_t **data);

private:
    std::unique_ptr<gs_texture_private> d_ptr{};
};

std::shared_ptr<gs_texture> gs_texture_create(uint32_t width, uint32_t height, gs_color_format color_format, uint32_t levels, const uint8_t **data, uint32_t flags);

void gs_set_render_target(std::shared_ptr<gs_texture> tex, std::shared_ptr<gs_zstencil_buffer> zs);

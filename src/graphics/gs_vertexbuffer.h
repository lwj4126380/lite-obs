#pragma once

#include <memory>
#include "graphics.h"

struct gs_vertexbuffer_private;
class gs_vertexbuffer
{
public:
    gs_vertexbuffer(std::unique_ptr<gs_vb_data> data, uint32_t flags);
    ~gs_vertexbuffer();

    bool create_buffers();
private:
    std::unique_ptr<gs_vertexbuffer_private> d_ptr{};
};

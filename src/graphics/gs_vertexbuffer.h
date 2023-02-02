#pragma once

#include <memory>
#include <vector>
#include <stdint.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

struct gs_tvertarray {
    size_t width{};
    void *array{};
};

struct gs_vb_data {
    size_t num{};
    std::vector<glm::vec3> points{};
    std::vector<glm::vec3> normals{};
    std::vector<glm::vec3> tangents{};
    std::vector<uint32_t> colors{};

    size_t num_tex{};
    std::vector<gs_tvertarray> tvarray{};

    ~gs_vb_data() {
        for (int i = 0; i < num_tex; ++i) {
            free(tvarray[i].array);
        }
    }
};

struct gs_vertexbuffer_private;
class gs_vertexbuffer
{
public:
    gs_vertexbuffer();
    ~gs_vertexbuffer();

    bool gs_vertexbuffer_init_sprite();

    void gs_vertexbuffer_flush();
    void gs_vertexbuffer_flush_direct(const gs_vb_data *data);

private:
    bool create_buffers();
    void gs_vertexbuffer_flush_internal(const gs_vb_data *data);

private:
    std::unique_ptr<gs_vertexbuffer_private> d_ptr{};
};

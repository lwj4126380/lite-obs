#pragma once

#include <memory>
#include <glm/vec3.hpp>
#include <vector>

#include "gs_subsystem_info.h"

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

struct graphics_subsystem_private;
class graphics_subsystem
{
public:
    graphics_subsystem();
    ~graphics_subsystem();

    int gs_create();

private:
    bool graphics_init();
    bool graphics_init_sprite_vb();

private:
    std::unique_ptr<graphics_subsystem_private> d_ptr{};
};

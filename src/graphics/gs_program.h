#pragma once

#include <memory>
#include "gs_shader.h"

struct gs_program_private;
class gs_program
{
public:
    gs_program();
    ~gs_program();

    bool gs_program_create(std::shared_ptr<gs_shader> vertex_shader, std::shared_ptr<gs_shader> pixel_shader);

private:
    bool assign_program_attrib(const shader_attrib &attrib);
    bool assign_program_attribs();

    bool assign_program_param(const std::shared_ptr<gs_shader_param> &param);
    bool assign_program_shader_params(std::shared_ptr<gs_shader> shader);
    bool assign_program_params();

private:
    std::unique_ptr<gs_program_private> d_ptr{};
};

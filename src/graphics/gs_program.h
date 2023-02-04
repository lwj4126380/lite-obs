#pragma once

#include <memory>
#include "gs_shader.h"

struct program_param;
struct gs_program_private;
class gs_texture;
class gs_program
{
public:
    gs_program();
    ~gs_program();

    bool gs_program_create(std::shared_ptr<gs_shader> vertex_shader, std::shared_ptr<gs_shader> pixel_shader);

    void gs_effect_set_texture(const char *name, std::shared_ptr<gs_texture> tex);

    void gs_effect_upload_parameters(bool change_only);

    std::shared_ptr<gs_shader> gs_effect_vertex_shader();
    std::shared_ptr<gs_shader> gs_effect_pixel_shader();

    GLuint gs_effect_obj();
    const std::vector<GLint> &gs_effect_attribs();

private:
    bool assign_program_attrib(const shader_attrib &attrib);
    bool assign_program_attribs();

    bool assign_program_param(const std::shared_ptr<gs_shader_param> &param);
    bool assign_program_shader_params(std::shared_ptr<gs_shader> shader);
    bool assign_program_params();

    program_param* gs_effect_get_param_by_name(const char *name);

private:
    std::unique_ptr<gs_program_private> d_ptr{};
};

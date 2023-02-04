#pragma once

#include <memory>
#include <vector>
#include "shaders.h"
#include "gs_subsystem_info.h"
#include "util/log.h"
#include <glm/mat4x4.hpp>

struct shader_attrib {
    std::string name{};
    size_t index{};
    attrib_type type{};
};

class gs_texture;
struct gs_shader_param {
    gs_shader_param_type type{};

    std::string name{};

    GLint texture_id{};
    size_t sampler_id{};
    int array_count{};

    std::weak_ptr<gs_texture> texture;

    std::vector<uint8_t> cur_value{};
    std::vector<uint8_t> def_value{};
    bool changed{};

    ~gs_shader_param() {
        blog(LOG_DEBUG, "gs_shader_param ------> destroy.");
    }
};

struct gs_sampler_state {
    GLint min_filter{};
    GLint mag_filter{};
    GLint address_u{};
    GLint address_v{};
    GLint address_w{};
};

struct gs_shader_private;
struct gs_shader_param;
class gs_shader
{
public:
    gs_shader();
    ~gs_shader();

    bool gs_shader_init(const gs_shader_info &info);

    GLuint obj();
    gs_shader_type type();

    const std::vector<shader_attrib> &gs_shader_attribs() const;
    const std::vector<std::shared_ptr<gs_shader_param>> &gs_shader_params() const;   
    std::shared_ptr<gs_shader_param> gs_shader_param_by_unit(int unit);

    void gs_shader_set_matrix4(const glm::mat4x4 &val);

private:
    std::string gl_get_shader_info(GLuint shader);
    bool gl_add_param(const gl_parser_shader_var &var, GLint *texture_id);
    bool gl_add_params(const std::vector<gl_parser_shader_var> &vars);
    bool gl_process_attribs(const std::vector<gl_parser_attrib> &attribs);
    bool gl_add_samplers(const std::vector<gl_parser_shader_sampler> &samplers);
    std::shared_ptr<gs_shader_param> gs_shader_get_param_by_name(const std::string &name);


private:
    std::unique_ptr<gs_shader_private> d_ptr{};
};

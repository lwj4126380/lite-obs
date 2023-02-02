#pragma once

#include <memory>
#include <vector>
#include "shaders.h"
#include "gs_subsystem_info.h"
#include "util/log.h"

enum class attrib_type {
    ATTRIB_POSITION,
    ATTRIB_NORMAL,
    ATTRIB_TANGENT,
    ATTRIB_COLOR,
    ATTRIB_TEXCOORD,
    ATTRIB_TARGET
};

struct shader_attrib {
    std::string name{};
    size_t index{};
    attrib_type type{};
};

struct gs_shader_param {
    gs_shader_param_type type{};

    std::string name{};
    //    gs_shader_t *shader;
    //    gs_samplerstate_t *next_sampler;
    GLint texture_id{};
    size_t sampler_id{};
    int array_count{};

    //    struct gs_texture *texture;

    std::vector<uint8_t> cur_value{};
    std::vector<uint8_t> def_value{};
    bool changed{};

    ~gs_shader_param() {
        blog(LOG_DEBUG, "gs_shader_param ------> destroy.");
    }
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

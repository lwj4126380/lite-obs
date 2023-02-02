#include "gs_program.h"
#include "gs_subsystem_info.h"
#include "gl-helpers.h"
#include "util/log.h"

#include <vector>

struct program_param {
    GLint obj{};
    std::shared_ptr<gs_shader_param> param{};
};

struct gs_program_private
{
    GLuint obj{};
    std::shared_ptr<gs_shader> vertex_shader{};
    std::shared_ptr<gs_shader> pixel_shader{};

    std::vector<program_param> params{};
    std::vector<GLint> attribs{};
};

gs_program::gs_program()
{
    d_ptr = std::make_unique<gs_program_private>();
}

gs_program::~gs_program()
{
    glDeleteProgram(d_ptr->obj);
    gl_success("glDeleteProgram");
}

static void print_link_errors(GLuint program)
{
    char *errors = NULL;
    GLint info_len = 0;
    GLsizei chars_written = 0;

    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_len);
    if (!gl_success("glGetProgramiv") || !info_len)
        return;

    errors = (char *)calloc(1, info_len + 1);
    glGetProgramInfoLog(program, info_len, &chars_written, errors);
    gl_success("glGetShaderInfoLog");

    blog(LOG_DEBUG, "Linker warnings/errors:\n%s", errors);

    free(errors);
}

bool gs_program::gs_program_create(std::shared_ptr<gs_shader> vertex_shader, std::shared_ptr<gs_shader> pixel_shader)
{
    int linked = false;

    d_ptr->vertex_shader = vertex_shader;
    d_ptr->pixel_shader = pixel_shader;

    d_ptr->obj = glCreateProgram();
    if (!gl_success("glCreateProgram"))
        goto error_detach_neither;

    glAttachShader(d_ptr->obj, d_ptr->vertex_shader->obj());
    if (!gl_success("glAttachShader (vertex)"))
        goto error_detach_neither;

    glAttachShader(d_ptr->obj, d_ptr->pixel_shader->obj());
    if (!gl_success("glAttachShader (pixel)"))
        goto error_detach_vertex;

    glLinkProgram(d_ptr->obj);
    if (!gl_success("glLinkProgram"))
        goto error;

    glGetProgramiv(d_ptr->obj, GL_LINK_STATUS, &linked);
    if (!gl_success("glGetProgramiv"))
        goto error;

    if (linked == GL_FALSE) {
        print_link_errors(d_ptr->obj);
        goto error;
    }

    if (!assign_program_attribs())
        goto error;
    if (!assign_program_params())
        goto error;

    glDetachShader(d_ptr->obj, d_ptr->vertex_shader->obj());
    gl_success("glDetachShader (vertex)");

    glDetachShader(d_ptr->obj, d_ptr->pixel_shader->obj());
    gl_success("glDetachShader (pixel)");

    return true;

error:
    glDetachShader(d_ptr->obj, d_ptr->pixel_shader->obj());
    gl_success("glDetachShader (pixel)");

error_detach_vertex:
    glDetachShader(d_ptr->obj, d_ptr->vertex_shader->obj());
    gl_success("glDetachShader (vertex)");

error_detach_neither:
    return false;
}

bool gs_program::assign_program_attrib(const shader_attrib &attrib)
{
    GLint attrib_obj = glGetAttribLocation(d_ptr->obj, attrib.name.c_str());
    if (!gl_success("glGetAttribLocation"))
        return false;

    if (attrib_obj == -1) {
        blog(LOG_ERROR,
             "glGetAttribLocation: Could not find "
             "attribute '%s'",
             attrib.name.c_str());
        return false;
    }

    d_ptr->attribs.push_back(attrib_obj);
    return true;
}

bool gs_program::assign_program_attribs()
{
    auto attribs = d_ptr->vertex_shader->gs_shader_attribs();
    for (int i = 0; i < attribs.size(); i++) {
        auto attrib = attribs[i];
        if (!assign_program_attrib(attrib))
            return false;
    }

    return true;
}

bool gs_program::assign_program_param(const std::shared_ptr<gs_shader_param> &param)
{
    struct program_param info;

    info.obj = glGetUniformLocation(d_ptr->obj, param->name.c_str());
    if (!gl_success("glGetUniformLocation"))
        return false;

    if (info.obj == -1) {
        return true;
    }

    info.param = param;
    d_ptr->params.push_back(std::move(info));
    return true;
}

bool gs_program::assign_program_shader_params(std::shared_ptr<gs_shader> shader)
{
    auto params = shader->gs_shader_params();
    for (int i = 0; i < params.size(); i++) {
        auto param = params[i];
        if (!assign_program_param(param))
            return false;
    }

    return true;
}

bool gs_program::assign_program_params()
{
    if (!assign_program_shader_params(d_ptr->vertex_shader))
        return false;
    if (!assign_program_shader_params(d_ptr->pixel_shader))
        return false;

    return true;
}

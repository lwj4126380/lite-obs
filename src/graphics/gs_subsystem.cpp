#include "gs_subsystem.h"
#include <mutex>
#include "gs_device.h"
#include "gs_vertexbuffer.h"
#include "gs_texture.h"
#include "gs_shader.h"
#include "shaders.h"
#include "gs_program.h"

#include "util/log.h"

#include <list>
#include <map>
#include <algorithm>
#include <glm/mat4x4.hpp>

std::vector<std::string> split (const std::string &s, std::string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find (delimiter, pos_start)) != std::string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}

struct blend_state {
    bool enabled{};
    gs_blend_type src_c{};
    gs_blend_type dest_c{};
    gs_blend_type src_a{};
    gs_blend_type dest_a{};
};

struct graphics_subsystem_private
{
    std::shared_ptr<gs_device> device{};

    std::list<gs_rect> viewport_stack{};

    std::list<glm::mat4x4> matrix_stack{};
    size_t cur_matrix{};

    glm::mat4x4 projection{0};

    std::shared_ptr<gs_vertexbuffer> sprite_buffer{};

    std::mutex effect_mutex;
    std::map<std::string, std::shared_ptr<gs_program>> effects{};

    blend_state cur_blend_state{};
    std::list<blend_state> blend_state_stack{};

    std::recursive_mutex mutex;
    std::atomic_long ref{};

    ~graphics_subsystem_private() {
        device->device_enter_context();
        sprite_buffer.reset();
        effects.clear();
        device->device_destroy();
        device->device_leave_context();
        device.reset();
    }
};

static thread_local graphics_subsystem *thread_graphics = NULL;

bool gs_valid(const char *f)
{
    if (!thread_graphics) {
        blog(LOG_DEBUG, "%s: called while not in a graphics context",
             f);
        return false;
    }

    return true;
}

std::unique_ptr<graphics_subsystem> gs_create_graphics_system()
{
    auto gs = std::make_unique<graphics_subsystem>();
    if (!gs->graphics_init())
        return nullptr;

    return gs;
}

graphics_subsystem::graphics_subsystem()
{
    d_ptr = std::make_unique<graphics_subsystem_private>();
}

graphics_subsystem::~graphics_subsystem()
{
    while (thread_graphics)
        gs_leave_context();

    if (d_ptr->device) {

        thread_graphics = this;
        d_ptr.reset();
        thread_graphics = nullptr;
    }

    blog(LOG_DEBUG, "graphics_subsystem destroyed.");
}

bool graphics_subsystem::init_effect()
{
    conversion_shaders_total.erase(std::remove(conversion_shaders_total.begin(), conversion_shaders_total.end(), '\n'), conversion_shaders_total.end());

    auto strs = split(conversion_shaders_total, "=======================================");
    for (int i = 0; i< strs.size(); i+=2) {
        auto &v = strs[i];
        auto &p = strs[i+1];
        std::string name;
        auto vertex_shader = shader_from_string(v, true, name);
        auto pixel_shader = shader_from_string(p, false, name);

        if (vertex_shader && pixel_shader) {
            auto program = std::make_shared<gs_program>(name);
            if (program->gs_program_create(vertex_shader, pixel_shader)) {
                d_ptr->effects.insert({name, program});
                blog(LOG_DEBUG, "gs program create: %s.", name.c_str());
            } else {
                return false;
                blog(LOG_DEBUG, "effect %s init error!", name.c_str());
            }
        } else {
            blog(LOG_DEBUG, "create shader error.");
            return false;
        }
    }

    return true;
}

std::shared_ptr<gs_program> graphics_subsystem::gs_get_effect_by_name(const char *name)
{
    if (d_ptr->effects.contains(name))
        return d_ptr->effects.at(name);

    return nullptr;
}

void graphics_subsystem::gs_draw_sprite(std::shared_ptr<gs_texture> tex, uint32_t flip, uint32_t width, uint32_t height)
{
    if (!tex && (!width || !height)) {
        blog(LOG_ERROR, "A sprite cannot be drawn without "
                        "a width/height");
        return;
    }

    float fcx = width ? (float)width : (float)tex->gs_texture_get_width();
    float fcy = height ? (float)height : (float)tex->gs_texture_get_height();

    auto data = d_ptr->sprite_buffer->gs_vertexbuffer_get_data();
    data->build_sprite_norm(fcx, fcy, flip);
    d_ptr->sprite_buffer->gs_vertexbuffer_flush();

    d_ptr->device->gs_device_load_vertexbuffer(d_ptr->sprite_buffer);
    d_ptr->device->gs_device_load_indexbuffer(nullptr);

    d_ptr->device->gs_device_draw(gs_draw_mode::GS_TRISTRIP, 0, 0);
}

bool graphics_subsystem::graphics_init()
{
    bool res = false;
    do {
        d_ptr->device = gs_create_device();
        if (!d_ptr->device)
            break;

        glm::mat4x4 top_mat(1);
        d_ptr->matrix_stack.push_back(top_mat);

        d_ptr->device->device_enter_context();

        if (!init_sprite_vb())
            break;

        if (!init_effect())
            break;

        d_ptr->device->device_blend_function_separate(gs_blend_type::GS_BLEND_SRCALPHA,
                                                      gs_blend_type::GS_BLEND_INVSRCALPHA,
                                                      gs_blend_type::GS_BLEND_ONE,
                                                      gs_blend_type::GS_BLEND_INVSRCALPHA);
        d_ptr->cur_blend_state.enabled = true;
        d_ptr->cur_blend_state.src_c = gs_blend_type::GS_BLEND_SRCALPHA;
        d_ptr->cur_blend_state.dest_c = gs_blend_type::GS_BLEND_INVSRCALPHA;
        d_ptr->cur_blend_state.src_a = gs_blend_type::GS_BLEND_ONE;
        d_ptr->cur_blend_state.dest_a = gs_blend_type::GS_BLEND_INVSRCALPHA;

        res = true;
    } while (false);

    if (d_ptr->device) {
        d_ptr->device->device_leave_context();
    }

    return res;
}

bool graphics_subsystem::init_sprite_vb()
{
    auto vb = std::make_shared<gs_vertexbuffer>();
    if (!vb->gs_vertexbuffer_init_sprite())
        return false;

    d_ptr->sprite_buffer = std::move(vb);
    return true;
}

std::shared_ptr<gs_shader> graphics_subsystem::shader_from_string(const std::string &shader_string, bool vertex_shader, std::string &out_name)
{
    auto strings = split(shader_string, "---------------------------------------");
    if (strings.size() != 5)
        return nullptr;

    auto &shader_name = strings[0];
    auto &shader_source = strings[1];
    auto &shader_param = strings[2];
    auto &shader_attribs = strings[3];
    auto &shader_samplers = strings[4];

    gs_shader_info shader_info;
    shader_info.shader = shader_source;
    shader_info.type = vertex_shader ? gs_shader_type::GS_SHADER_VERTEX : gs_shader_type::GS_SHADER_PIXEL;

    if (shader_param.length() > 0) {
        auto params = split(shader_param, "+++++++++++++++++++++++++++++++++++++++");
        for (int i = 0; i < params.size(); ++i) {
            auto &param = params[i];
            auto p = split(param, " ");
            if (p.size() != 6)
                return nullptr;

            gl_parser_shader_var var;
            var.type = p[0];
            var.name = p[1];
            var.mapping = p[2] == "null" ? "" : p[2];
            var.var_type = (shader_var_type)std::stoi(p[3]);
            var.gl_sampler_id = std::stoull(p[5]);

            shader_info.parser_shader_vars.push_back(std::move(var));
        }
    }

    if (shader_attribs.length() > 0) {
        auto attribs = split(shader_attribs, "+++++++++++++++++++++++++++++++++++++++");
        for (int i = 0; i < attribs.size(); ++i) {
            auto &attrib = attribs[i];
            auto a = split(attrib, " ");
            if (a.size() != 3)
                return nullptr;

            gl_parser_attrib att;
            att.name = a[0];
            att.mapping = a[1] == "null" ? "" : a[1];
            att.input = std::stoi(a[2]);

            shader_info.parser_attribs.push_back(std::move(att));
        }
    }

    if (shader_samplers.length() > 0) {
        auto samplers = split(shader_samplers, "+++++++++++++++++++++++++++++++++++++++");
        for (int i = 0; i < samplers.size(); ++i) {
            auto name = samplers[i];
            if (name == "def_sampler") {
                gl_parser_shader_sampler sampler;
                sampler.name = "def_sampler";
                sampler.states = {"Filter", "AddressU", "AddressV"};
                sampler.values = {"Linear", "Clamp", "Clamp"};

                shader_info.parser_shader_samplers.push_back(std::move(sampler));
            } else if (name == "textureSampler") {
                gl_parser_shader_sampler sampler;
                sampler.name = "textureSampler";
                sampler.states = {"Filter", "AddressU", "AddressV"};
                sampler.values = {"Linear", "Clamp", "Clamp"};

                shader_info.parser_shader_samplers.push_back(std::move(sampler));
            }
        }
    }

    auto shader = std::make_shared<gs_shader>();
    if (!shader->gs_shader_init(shader_info))
        return nullptr;

    out_name = shader_name;
    return shader;
}


void gs_enter_contex(std::unique_ptr<graphics_subsystem> &graphics)
{
    bool is_current = thread_graphics == graphics.get();
    if (thread_graphics && !is_current) {
        while (thread_graphics)
            gs_leave_context();
    }

    if (!is_current) {
        graphics->d_ptr->mutex.lock();
        graphics->d_ptr->device->device_enter_context();
        thread_graphics = graphics.get();
    }

    graphics->d_ptr->ref++;
}

void gs_leave_context()
{
    if (gs_valid("gs_leave_context")) {
        if (!--thread_graphics->d_ptr->ref) {
            graphics_subsystem *graphics = thread_graphics;

            graphics->d_ptr->device->device_leave_context();
            graphics->d_ptr->mutex.unlock();
            thread_graphics = nullptr;
        }
    }
}

graphics_subsystem *gs_graphics_subsystem()
{
    return thread_graphics;
}

void gs_set_render_target(std::shared_ptr<gs_texture> tex, std::shared_ptr<gs_zstencil_buffer> zs)
{
    if (!gs_valid("gs_set_render_target"))
        return;

    if (!thread_graphics->d_ptr->device->gs_device_set_render_target(tex, zs))
        blog(LOG_ERROR, "device_set_render_target (GL) failed");
}

void gs_begin_scene()
{
    if (!gs_valid("gs_begin_scene"))
        return;

    thread_graphics->d_ptr->device->gs_device_clear_textures();
}

void gs_end_scene()
{
    /*do nothing*/
}

void gs_enable_depth_test(bool enable)
{
    if (!gs_valid("gs_enable_depth_test"))
        return;

    if (enable)
        gl_enable(GL_DEPTH_TEST);
    else
        gl_disable(GL_DEPTH_TEST);
}

void gs_enable_blending(bool enable)
{
    if (!gs_valid("gs_enable_blending"))
        return;

    thread_graphics->d_ptr->cur_blend_state.enabled = enable;

    if (enable)
        gl_enable(GL_BLEND);
    else
        gl_disable(GL_BLEND);
}

void gs_set_cull_mode(gs_cull_mode mode)
{
    if (!gs_valid("gs_set_cull_mode"))
        return;

    thread_graphics->d_ptr->device->gs_device_set_cull_mode(mode);
}

void gs_ortho(float left, float right, float top, float bottom, float znear, float zfar)
{
    if (!gs_valid("gs_ortho"))
        return;

    thread_graphics->d_ptr->device->gs_device_ortho(left, right, top, bottom, znear, zfar);
}

void gs_set_viewport(int x, int y, int width, int height)
{
    if (!gs_valid("gs_set_view_port"))
        return;

    thread_graphics->d_ptr->device->gs_device_set_viewport(x, y, width, height);
}

void gs_get_viewport(gs_rect &rect)
{
    if (!gs_valid("gs_get_viewport"))
        return;

    thread_graphics->d_ptr->device->gs_device_get_viewport(rect);
}

void gs_clear(uint32_t clear_flags, glm::vec4 *color, float depth, uint8_t stencil)
{
    if (!gs_valid("gs_clear"))
        return;

    GLbitfield gl_flags = 0;

    if (clear_flags & GS_CLEAR_COLOR) {
        glClearColor(color->x, color->y, color->z, color->w);
        gl_flags |= GL_COLOR_BUFFER_BIT;
    }

    if (clear_flags & GS_CLEAR_DEPTH) {
#if defined __ANDROID__
        glClearDepthf(depth);
#else
        glClearDepth(depth);
#endif
        gl_flags |= GL_DEPTH_BUFFER_BIT;
    }

    if (clear_flags & GS_CLEAR_STENCIL) {
        glClearStencil(stencil);
        gl_flags |= GL_STENCIL_BUFFER_BIT;
    }

    glClear(gl_flags);
    if (!gl_success("glClear"))
        blog(LOG_ERROR, "device_clear (GL) failed");
}

void gs_set_render_size(uint32_t width, uint32_t height)
{
    if (!gs_valid("gs_set_render_size"))
        return;

    gs_enable_depth_test(false);
    gs_set_cull_mode(gs_cull_mode::GS_NEITHER);

    gs_ortho(0.0f, (float)width, 0.0f, (float)height, -100.0f, 100.0f);
    gs_set_viewport(0, 0, width, height);
}

void gs_load_texture(std::weak_ptr<gs_texture> tex, int unit)
{
    if (!gs_valid("gs_load_texture"))
        return;

    thread_graphics->d_ptr->device->gs_device_load_texture(tex, unit);
}

void gs_matrix_get(glm::mat4x4 &matrix)
{
    if (!gs_valid("gs_matrix_get"))
        return;

    matrix = thread_graphics->d_ptr->matrix_stack.back();
}

void gs_set_cur_effect(std::shared_ptr<gs_program> program)
{
    if (!gs_valid("gs_set_cur_effect"))
        return;

    thread_graphics->d_ptr->device->gs_device_set_program(program);
}

void gs_technique_begin()
{
    if (!gs_valid("gs_technique_begin"))
        return;

    thread_graphics->d_ptr->device->gs_device_clear_textures();
    thread_graphics->d_ptr->device->gs_device_load_default_pixelshader_samplers();
    auto program = thread_graphics->d_ptr->device->gs_device_program();
    if (!program)
        return;

    program->gs_effect_upload_parameters(false);
}

void gs_technique_end()
{
    if (!gs_valid("gs_technique_end"))
        return;

    auto program = thread_graphics->d_ptr->device->gs_device_program();
    if (!program)
        return;

    program->gs_effect_clear_tex_params();
    program->gs_effect_clear_all_params();
    thread_graphics->d_ptr->device->gs_device_clear_textures();
    thread_graphics->d_ptr->device->gs_device_set_program(nullptr);
}

void gs_draw(gs_draw_mode draw_mode, uint32_t start_vert, uint32_t num_verts)
{
    if (!gs_valid("gs_draw"))
        return;

    thread_graphics->d_ptr->device->gs_device_draw(draw_mode, start_vert, num_verts);
}

void gs_viewport_push()
{
    if (!gs_valid("gs_viewport_push"))
        return;

    gs_rect rect{};
    gs_get_viewport(rect);
    thread_graphics->d_ptr->viewport_stack.push_back(rect);
}

void gs_viewport_pop()
{
    if (!gs_valid("gs_viewport_pop"))
        return;

    auto &viewport = thread_graphics->d_ptr->viewport_stack;
    if (viewport.empty())
        return;

    auto rect = viewport.back();
    viewport.pop_back();
    gs_set_viewport(rect.x, rect.y, rect.cx, rect.cy);
}

void gs_projection_push()
{
    if (!gs_valid("gs_projection_push"))
        return;

    thread_graphics->d_ptr->device->gs_device_projection_push();
}

void gs_projection_pop()
{
    if (!gs_valid("gs_projection_pop"))
        return;

    thread_graphics->d_ptr->device->gs_device_projection_pop();
}

void gs_matrix_push()
{
    if (!gs_valid("gs_matrix_push"))
        return;

    auto mat = thread_graphics->d_ptr->matrix_stack.back();
    thread_graphics->d_ptr->matrix_stack.push_back(mat);
}

void gs_matrix_pop()
{
    if (!gs_valid("gs_matrix_pop"))
        return;

    auto &stack = thread_graphics->d_ptr->matrix_stack;
    if (stack.empty()) {
        blog(LOG_ERROR, "Tried to pop last matrix on stack");
        return;
    }

    stack.pop_back();
}

void gs_matrix_identity()
{
    if (!gs_valid("gs_matrix_identity"))
        return;

    auto &stack = thread_graphics->d_ptr->matrix_stack;
    if (stack.empty()) {
        return;
    }

    auto &mat = stack.back();
    mat = glm::mat4x4{1};
}

std::shared_ptr<gs_texture> gs_get_render_target()
{
    if (!gs_valid("gs_get_render_target"))
        return nullptr;

    return thread_graphics->d_ptr->device->gs_device_get_render_target();
}

std::shared_ptr<gs_zstencil_buffer> gs_get_zstencil_target()
{
    if (!gs_valid("gs_get_zstencil_target"))
        return nullptr;

    return thread_graphics->d_ptr->device->gs_device_get_zstencil_target();
}

void gs_flush()
{
    if (!gs_valid("gs_flush"))
        return;

    glFlush();
}

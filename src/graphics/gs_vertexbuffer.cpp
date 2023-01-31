#include "gs_vertexbuffer.h"
#include "gl-helpers.h"
#include <stdint.h>

struct gs_vertexbuffer_private
{
    GLuint vao{};
    GLuint vertex_buffer{};
    GLuint normal_buffer{};
    GLuint tangent_buffer{};
    GLuint color_buffer{};
    std::vector<GLuint> uv_buffers{};
    std::vector<size_t> uv_sizes{};

    size_t num{};
    bool dynamic{};
    std::unique_ptr<gs_vb_data> data{};
};

gs_vertexbuffer::gs_vertexbuffer(std::unique_ptr<gs_vb_data> data, uint32_t flags)
{
    d_ptr = std::make_unique<gs_vertexbuffer_private>();
    d_ptr->num = data->num;
    d_ptr->dynamic = flags & GS_DYNAMIC;
    d_ptr->data = std::move(data);
}

gs_vertexbuffer::~gs_vertexbuffer()
{

}

bool gs_vertexbuffer::create_buffers()
{
    GLenum usage = d_ptr->dynamic ? GL_STREAM_DRAW : GL_STATIC_DRAW;
    size_t i;

    if (!gl_create_buffer(GL_ARRAY_BUFFER, &d_ptr->vertex_buffer,
                          d_ptr->data->num * sizeof(glm::vec3),
                          d_ptr->data->points.data(), usage))
        return false;

    if (!d_ptr->data->normals.empty()) {
        if (!gl_create_buffer(GL_ARRAY_BUFFER, &d_ptr->normal_buffer,
                              d_ptr->data->num * sizeof(glm::vec3),
                              d_ptr->data->normals.data(), usage))
            return false;
    }

    if (!d_ptr->data->tangents.empty()) {
        if (!gl_create_buffer(GL_ARRAY_BUFFER, &d_ptr->tangent_buffer,
                              d_ptr->data->num * sizeof(glm::vec3),
                              d_ptr->data->tangents.data(), usage))
            return false;
    }

    if (!d_ptr->data->colors.empty()) {
        if (!gl_create_buffer(GL_ARRAY_BUFFER, &d_ptr->color_buffer,
                              d_ptr->data->num * sizeof(uint32_t),
                              d_ptr->data->colors.data(), usage))
            return false;
    }

    d_ptr->uv_buffers.resize(d_ptr->data->num_tex);
    d_ptr->uv_sizes.resize(d_ptr->data->num_tex);

    for (i = 0; i < d_ptr->data->num_tex; i++) {
        GLuint tex_buffer;
        gs_tvertarray &tv = d_ptr->data->tvarray[i];
        size_t size = d_ptr->data->num * sizeof(float) * tv.width;

        if (!gl_create_buffer(GL_ARRAY_BUFFER, &tex_buffer, size,
                              tv.array, usage))
            return false;

        d_ptr->uv_buffers[i] = tex_buffer;
        d_ptr->uv_sizes[i] = tv.width;
    }

    if (!d_ptr->dynamic) {
        d_ptr->data.reset();
    }

    if (!gl_gen_vertex_arrays(1, &d_ptr->vao))
        return false;

    return true;
}

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

gs_vertexbuffer::gs_vertexbuffer()
{
    d_ptr = std::make_unique<gs_vertexbuffer_private>();
}

gs_vertexbuffer::~gs_vertexbuffer()
{
    if (d_ptr->vertex_buffer)
        gl_delete_buffers(1, &d_ptr->vertex_buffer);
    if (d_ptr->normal_buffer)
        gl_delete_buffers(1, &d_ptr->normal_buffer);
    if (d_ptr->tangent_buffer)
        gl_delete_buffers(1, &d_ptr->tangent_buffer);
    if (d_ptr->color_buffer)
        gl_delete_buffers(1, &d_ptr->color_buffer);
    if (d_ptr->uv_buffers.size())
        gl_delete_buffers((GLsizei)d_ptr->uv_buffers.size(),
                          d_ptr->uv_buffers.data());

    if (d_ptr->vao)
        gl_delete_vertex_arrays(1, &d_ptr->vao);
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

bool gs_vertexbuffer::gs_vertexbuffer_init_sprite()
{
    d_ptr->data = std::make_unique<gs_vb_data>();
    d_ptr->data->num = 4;
    d_ptr->data->points.resize(4);
    d_ptr->data->num_tex = 1;
    d_ptr->data->tvarray.resize(1);
    d_ptr->data->tvarray[0].width = 2;
    d_ptr->data->tvarray[0].array = malloc(sizeof(glm::vec2) * 4);
    memset(d_ptr->data->tvarray[0].array, 0, sizeof(glm::vec2) * 4);

    d_ptr->num = d_ptr->data->num;
    d_ptr->dynamic = true;

    return create_buffers();
}

void gs_vertexbuffer::gs_vertexbuffer_flush()
{
    gs_vertexbuffer_flush_internal(d_ptr->data.get());
}

void gs_vertexbuffer::gs_vertexbuffer_flush_direct(const gs_vb_data *data)
{
    gs_vertexbuffer_flush_internal(data);
}

void gs_vertexbuffer::gs_vertexbuffer_flush_internal(const gs_vb_data *data)
{
    size_t i;
    size_t num_tex = data->num_tex < d_ptr->data->num_tex ? data->num_tex
                                                       : d_ptr->data->num_tex;

    if (!d_ptr->dynamic) {
        blog(LOG_ERROR, "vertex buffer is not dynamic");
        goto failed;
    }

    if (data->points.size()) {
        if (!update_buffer(GL_ARRAY_BUFFER, d_ptr->vertex_buffer,
                           data->points.data(),
                           data->num * sizeof(glm::vec3)))
            goto failed;
    }

    if (d_ptr->normal_buffer && data->normals.size()) {
        if (!update_buffer(GL_ARRAY_BUFFER, d_ptr->normal_buffer,
                           data->normals.data(),
                           data->num * sizeof(glm::vec3)))
            goto failed;
    }

    if (d_ptr->tangent_buffer && data->tangents.size()) {
        if (!update_buffer(GL_ARRAY_BUFFER, d_ptr->tangent_buffer,
                           data->tangents.data(),
                           data->num * sizeof(glm::vec3)))
            goto failed;
    }

    if (d_ptr->color_buffer && data->colors.size()) {
        if (!update_buffer(GL_ARRAY_BUFFER, d_ptr->color_buffer,
                           data->colors.data(), data->num * sizeof(uint32_t)))
            goto failed;
    }

    for (i = 0; i < num_tex; i++) {
        GLuint buffer = d_ptr->uv_buffers[i];
        const gs_tvertarray &tv = data->tvarray[i];
        size_t size = data->num * tv.width * sizeof(float);

        if (!update_buffer(GL_ARRAY_BUFFER, buffer, tv.array, size))
            goto failed;
    }

    return;

failed:
    blog(LOG_ERROR, "gs_vertexbuffer_flush (GL) failed");
}

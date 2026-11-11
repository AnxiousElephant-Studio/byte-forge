#include <byte-forge-engine/modules/rendering/opengl/mesh/opengl_mesh.h>

#include <glad/gl.h>

#include <iterator>

namespace byteforge::rendering
{
    namespace
    {
        const f32 quad_vertices[] =
        {
            -0.5f,  0.5f, 0.0f,   0.0f, 1.0f,   0.0f, 0.0f, 1.0f,
             0.5f,  0.5f, 0.0f,   1.0f, 1.0f,   0.0f, 0.0f, 1.0f,
             0.5f, -0.5f, 0.0f,   1.0f, 0.0f,   0.0f, 0.0f, 1.0f,
            -0.5f, -0.5f, 0.0f,   0.0f, 0.0f,   0.0f, 0.0f, 1.0f,
        };

        const u32 quad_indices[] = { 0, 1, 2, 2, 3, 0 };

        const u32 quad_attribute_sizes[] = { 3, 2, 3 };
    }

    bool opengl_mesh_t::create(const f32* vertices, usize vertex_float_count,
                               const u32* indices, usize index_count,
                               const u32* attribute_sizes, usize attribute_count)
    {
        u32 vertex_float_stride = 0;

        for (usize index = 0; index < attribute_count; index++)
        {
            vertex_float_stride += attribute_sizes[index];
        }

        const GLsizei vertex_stride = static_cast<GLsizei>(vertex_float_stride * sizeof(f32));

        glGenVertexArrays(1, &vertex_array_id);
        glGenBuffers(1, &vertex_buffer_id);
        glGenBuffers(1, &index_buffer_id);

        glBindVertexArray(vertex_array_id);

        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertex_float_count * sizeof(f32)), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(index_count * sizeof(u32)), indices, GL_STATIC_DRAW);

        usize attribute_offset = 0;

        for (usize index = 0; index < attribute_count; index++)
        {
            glVertexAttribPointer(static_cast<GLuint>(index), static_cast<GLint>(attribute_sizes[index]), GL_FLOAT, GL_FALSE, vertex_stride, reinterpret_cast<void*>(attribute_offset * sizeof(f32)));
            glEnableVertexAttribArray(static_cast<GLuint>(index));

            attribute_offset += attribute_sizes[index];
        }

        glBindVertexArray(0);

        this->index_count = static_cast<u32>(index_count);

        return true;
    }

    bool opengl_mesh_t::create_quad()
    {
        return create(quad_vertices, std::size(quad_vertices), quad_indices, std::size(quad_indices), quad_attribute_sizes, std::size(quad_attribute_sizes));
    }

    void opengl_mesh_t::draw() const
    {
        glBindVertexArray(vertex_array_id);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(index_count), GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }

    void opengl_mesh_t::dispose()
    {
        glDeleteVertexArrays(1, &vertex_array_id);
        glDeleteBuffers(1, &vertex_buffer_id);
        glDeleteBuffers(1, &index_buffer_id);

        vertex_array_id  = 0;
        vertex_buffer_id = 0;
        index_buffer_id  = 0;
        index_count      = 0;
    }
}

#include <byte-forge-engine/modules/rendering/opengl/shader/opengl_shader.h>
#include <byte-forge-engine/modules/io/file.h>
#include <byte-forge-engine/modules/io/e_file_data_types.h>
#include <byte-forge-engine/modules/logging/log_macros.h>

#include <glad/gl.h>

namespace byteforge::rendering
{
    namespace
    {
        const i32 info_log_size = 1024;

        const char* const embedded_source_label = "<embedded>";

        const char* const read_error_format    = "Could not read shader at path: '%s'.";
        const char* const compile_error_format = "Could not compile shader at path: '%s'. Cause: %s";
        const char* const link_error_format    = "Could not link shader program with vertex shader at path: '%s' and fragment shader at path: '%s'. Cause: %s";

        u32 compile_shader_source(GLenum shader_type, const char* source, const char* label)
        {
            u32 shader_id = glCreateShader(shader_type);

            glShaderSource(shader_id, 1, &source, nullptr);
            glCompileShader(shader_id);

            i32 compiled = 0;
            glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled);

            if (compiled == 0)
            {
                char info_log[info_log_size];
                glGetShaderInfoLog(shader_id, info_log_size, nullptr, info_log);
                LOG_ERROR(compile_error_format, label, info_log);

                glDeleteShader(shader_id);
                return 0;
            }

            return shader_id;
        }

        u32 compile_shader_file(GLenum shader_type, const char* path)
        {
            io::file_t file;

            if (!file.read(path, io::e_file_data_types::Text))
            {
                LOG_ERROR(read_error_format, path);
                return 0;
            }

            return compile_shader_source(shader_type, file.content, path);
        }
    }

    bool opengl_shader_t::create(const char* vertex_shader_path, const char* fragment_shader_path)
    {
        return link(compile_shader_file(GL_VERTEX_SHADER, vertex_shader_path), compile_shader_file(GL_FRAGMENT_SHADER, fragment_shader_path), vertex_shader_path, fragment_shader_path);
    }

    bool opengl_shader_t::create_from_source(const char* vertex_shader_source, const char* fragment_shader_source)
    {
        return create_from_source(vertex_shader_source, fragment_shader_source, embedded_source_label, embedded_source_label);
    }

    bool opengl_shader_t::create_from_source(const char* vertex_shader_source, const char* fragment_shader_source, const char* vertex_shader_label, const char* fragment_shader_label)
    {
        return link(compile_shader_source(GL_VERTEX_SHADER, vertex_shader_source, vertex_shader_label), compile_shader_source(GL_FRAGMENT_SHADER, fragment_shader_source, fragment_shader_label), vertex_shader_label, fragment_shader_label);
    }

    bool opengl_shader_t::link(u32 vertex_id, u32 fragment_id, const char* vertex_label, const char* fragment_label)
    {
        if (vertex_id == 0 || fragment_id == 0)
        {
            glDeleteShader(vertex_id);
            glDeleteShader(fragment_id);
            return false;
        }

        program_id = glCreateProgram();
        glAttachShader(program_id, vertex_id);
        glAttachShader(program_id, fragment_id);
        glLinkProgram(program_id);

        glDeleteShader(vertex_id);
        glDeleteShader(fragment_id);

        i32 linked = 0;
        glGetProgramiv(program_id, GL_LINK_STATUS, &linked);

        if (linked == 0)
        {
            char info_log[info_log_size];
            glGetProgramInfoLog(program_id, info_log_size, nullptr, info_log);
            LOG_ERROR(link_error_format, vertex_label, fragment_label, info_log);

            dispose();
            return false;
        }

        return true;
    }

    void opengl_shader_t::bind() const
    {
        glUseProgram(program_id);
    }

    i32 opengl_shader_t::get_uniform_location(const char* name) const
    {
        return glGetUniformLocation(program_id, name);
    }

    void opengl_shader_t::set_matrix_4(i32 uniform_location, const math::mat4_t& matrix) const
    {
        glUniformMatrix4fv(uniform_location, 1, GL_FALSE, &matrix.values[0]);
    }

    void opengl_shader_t::set_vector_2(i32 uniform_location, const math::vec2_t& vector) const
    {
        glUniform2f(uniform_location, vector.x, vector.y);
    }

    void opengl_shader_t::set_vector_3(i32 uniform_location, const math::vec3_t& vector) const
    {
        glUniform3f(uniform_location, vector.x, vector.y, vector.z);
    }

    void opengl_shader_t::set_vector_4(i32 uniform_location, const math::vec4_t& vector) const
    {
        glUniform4f(uniform_location, vector.x, vector.y, vector.z, vector.w);
    }

    void opengl_shader_t::set_float(i32 uniform_location, f32 value) const
    {
        glUniform1f(uniform_location, value);
    }

    void opengl_shader_t::set_int(i32 uniform_location, i32 value) const
    {
        glUniform1i(uniform_location, value);
    }

    void opengl_shader_t::set_bool(i32 uniform_location, bool value) const
    {
        glUniform1i(uniform_location, value ? 1 : 0);
    }

    void opengl_shader_t::dispose()
    {
        glDeleteProgram(program_id);
        program_id = 0;
    }
}

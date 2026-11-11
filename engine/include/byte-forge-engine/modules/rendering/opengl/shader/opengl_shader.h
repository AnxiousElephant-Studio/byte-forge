#pragma once

#include <byte-forge-engine/modules/rendering/rendering_api.h>
#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/math/matrices/matrix4.h>
#include <byte-forge-engine/modules/math/vectors/vector2.h>
#include <byte-forge-engine/modules/math/vectors/vector3.h>
#include <byte-forge-engine/modules/math/vectors/vector4.h>

namespace byteforge::rendering
{
    struct BF_RENDERING_API opengl_shader_t
    {
    public:
        u32 program_id;

        opengl_shader_t() : program_id(0) {}

        bool create(const char* vertex_shader_path, const char* fragment_shader_path);
        bool create_from_source(const char* vertex_shader_source, const char* fragment_shader_source);
        bool create_from_source(const char* vertex_shader_source, const char* fragment_shader_source, const char* vertex_shader_label, const char* fragment_shader_label);
        void dispose();

        void bind() const;

        i32 get_uniform_location(const char* name) const;

        void set_matrix_4(i32 uniform_location, const math::mat4_t& matrix) const;
        void set_vector_2(i32 uniform_location, const math::vec2_t& vector) const;
        void set_vector_3(i32 uniform_location, const math::vec3_t& vector) const;
        void set_vector_4(i32 uniform_location, const math::vec4_t& vector) const;
        void set_float(i32 uniform_location, f32 value) const;
        void set_int(i32 uniform_location, i32 value) const;
        void set_bool(i32 uniform_location, bool value) const;

    private:
        bool link(u32 vertex_id, u32 fragment_id, const char* vertex_label, const char* fragment_label);
    };
}

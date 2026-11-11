#pragma once

#include <byte-forge-engine/modules/rendering/rendering_api.h>
#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/math/matrices/matrix4.h>
#include <byte-forge-engine/modules/math/vectors/vector2.h>
#include <byte-forge-engine/modules/math/vectors/vector3.h>
#include <byte-forge-engine/modules/math/vectors/vector4.h>
#include <byte-forge-engine/modules/rendering/material/e_material_shading_models.h>
#include <byte-forge-engine/modules/rendering/opengl/shader/opengl_shader.h>
#include <byte-forge-engine/modules/rendering/opengl/texture/opengl_texture.h>

#include <string>
#include <unordered_map>

namespace byteforge::rendering
{
    class BF_RENDERING_API material_t
    {
    public:
        material_t()
            : shader()
            , shading_model(e_material_shading_models::Unlit)
            , surface_path()
            , projection_location(-1)
            , model_location(-1)
            , time_location(-1)
            , light_map_location(-1)
            , parameter_locations()
        {
        }

        bool load(const char* material_surface_path, const e_material_shading_models material_shading_model);
        bool load_from_source(const char* surface_source, const char* source_label, const e_material_shading_models material_shading_model);
        bool reload();
        void dispose();

        void bind() const;

        void set_projection(const math::mat4_t& projection) const;
        void set_model(const math::mat4_t& model) const;
        void set_time(f32 time_in_seconds) const;
        void set_light_map(const opengl_texture_t& light_map, u32 texture_slot) const;

        void set_float(const char* parameter_name, f32 value);
        void set_int(const char* parameter_name, i32 value);
        void set_vector_2(const char* parameter_name, const math::vec2_t& value);
        void set_vector_3(const char* parameter_name, const math::vec3_t& value);
        void set_vector_4(const char* parameter_name, const math::vec4_t& value);
        void set_texture(const char* parameter_name, const opengl_texture_t& texture, u32 texture_slot);

        e_material_shading_models get_shading_model() const { return shading_model; }
        const opengl_shader_t& get_shader() const { return shader; }
        bool is_loaded() const { return shader.program_id != 0; }

    private:
        i32 find_parameter_location(const char* parameter_name);

        opengl_shader_t shader;
        e_material_shading_models shading_model;
        bool compile(const char* surface_source, const char* source_label);

        std::string surface_path;

        i32 projection_location;
        i32 model_location;
        i32 time_location;
        i32 light_map_location;

        std::unordered_map<std::string, i32> parameter_locations;
    };
}

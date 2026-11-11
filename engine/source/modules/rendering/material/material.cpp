#include <byte-forge-engine/modules/rendering/material/material.h>
#include <byte-forge-engine/modules/io/e_file_data_types.h>
#include <byte-forge-engine/modules/io/file.h>
#include <byte-forge-engine/modules/logging/log_macros.h>

namespace byteforge::rendering
{
    namespace
    {
        const char* const read_error_format = "Could not load the material at path: '%s'. Cause: the file could not be read.";

        const char* const vertex_template_label = "<material vertex template>";

        const char* const vertex_template = R"(#version 460 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_uv;
layout(location = 2) in vec3 a_normal;

uniform mat4 u_projection;
uniform mat4 u_model;

out vec2 v_uv;
out vec3 v_world_position;
out vec3 v_normal;

void main()
{
    const vec4 world_position = u_model * vec4(a_position, 1.0);

    v_uv             = a_uv;
    v_world_position = world_position.xyz;
    v_normal         = mat3(u_model) * a_normal;
    gl_Position      = u_projection * world_position;
}
)";

        const char* const fragment_header = R"(#version 460 core

in vec2 v_uv;
in vec3 v_world_position;
in vec3 v_normal;

uniform float u_time;

struct material_input_t
{
    vec2 uv;
    vec3 world_position;
    vec3 normal;
    float time;
};

struct material_output_t
{
    vec3 base_color;
    float opacity;
    vec3 emissive;
};

#line 1
)";

        const char* const fragment_surface_call = R"(

out vec4 out_color;

material_output_t run_surface()
{
    const material_input_t surface_input = material_input_t(v_uv, v_world_position, normalize(v_normal), u_time);
    material_output_t surface_output     = material_output_t(vec3(1.0), 1.0, vec3(0.0));

    surface(surface_input, surface_output);

    return surface_output;
}
)";

        const char* const unlit_main = R"(
void main()
{
    const material_output_t surface_output = run_surface();

    out_color = vec4(surface_output.base_color + surface_output.emissive, surface_output.opacity);
}
)";

        const char* const lit_main = R"(
uniform sampler2D u_light_map;

void main()
{
    const material_output_t surface_output = run_surface();
    const vec3 light = texture(u_light_map, gl_FragCoord.xy / vec2(textureSize(u_light_map, 0))).rgb;

    out_color = vec4(surface_output.base_color * light + surface_output.emissive, surface_output.opacity);
}
)";

        const char* get_shading_model_main(const e_material_shading_models shading_model)
        {
            return shading_model == e_material_shading_models::Lit ? lit_main : unlit_main;
        }
    }

    bool material_t::load(const char* material_surface_path, const e_material_shading_models material_shading_model)
    {
        surface_path  = material_surface_path;
        shading_model = material_shading_model;

        return reload();
    }

    bool material_t::load_from_source(const char* surface_source, const char* source_label, const e_material_shading_models material_shading_model)
    {
        surface_path.clear();
        shading_model = material_shading_model;

        return compile(surface_source, source_label);
    }

    bool material_t::reload()
    {
        if (surface_path.empty())
        {
            return is_loaded();
        }

        io::file_t surface_file;

        if (!surface_file.read(surface_path.c_str(), io::e_file_data_types::Text))
        {
            LOG_ERROR(read_error_format, surface_path.c_str());
            return false;
        }

        return compile(surface_file.content, surface_path.c_str());
    }

    bool material_t::compile(const char* surface_source, const char* source_label)
    {
        const std::string fragment_source = std::string(fragment_header) + surface_source + fragment_surface_call + get_shading_model_main(shading_model);

        opengl_shader_t new_shader;

        if (!new_shader.create_from_source(vertex_template, fragment_source.c_str(), vertex_template_label, source_label))
        {
            return false;
        }

        shader.dispose();
        shader = new_shader;
        parameter_locations.clear();

        projection_location = shader.get_uniform_location("u_projection");
        model_location      = shader.get_uniform_location("u_model");
        time_location       = shader.get_uniform_location("u_time");
        light_map_location  = shader.get_uniform_location("u_light_map");

        return true;
    }

    void material_t::dispose()
    {
        shader.dispose();
        parameter_locations.clear();
    }

    void material_t::bind() const
    {
        shader.bind();
    }

    void material_t::set_projection(const math::mat4_t& projection) const
    {
        shader.set_matrix_4(projection_location, projection);
    }

    void material_t::set_model(const math::mat4_t& model) const
    {
        shader.set_matrix_4(model_location, model);
    }

    void material_t::set_time(f32 time_in_seconds) const
    {
        shader.set_float(time_location, time_in_seconds);
    }

    void material_t::set_light_map(const opengl_texture_t& light_map, u32 texture_slot) const
    {
        light_map.bind(texture_slot);
        shader.set_int(light_map_location, static_cast<i32>(texture_slot));
    }

    void material_t::set_float(const char* parameter_name, f32 value)
    {
        shader.set_float(find_parameter_location(parameter_name), value);
    }

    void material_t::set_int(const char* parameter_name, i32 value)
    {
        shader.set_int(find_parameter_location(parameter_name), value);
    }

    void material_t::set_vector_2(const char* parameter_name, const math::vec2_t& value)
    {
        shader.set_vector_2(find_parameter_location(parameter_name), value);
    }

    void material_t::set_vector_3(const char* parameter_name, const math::vec3_t& value)
    {
        shader.set_vector_3(find_parameter_location(parameter_name), value);
    }

    void material_t::set_vector_4(const char* parameter_name, const math::vec4_t& value)
    {
        shader.set_vector_4(find_parameter_location(parameter_name), value);
    }

    void material_t::set_texture(const char* parameter_name, const opengl_texture_t& texture, u32 texture_slot)
    {
        texture.bind(texture_slot);
        shader.set_int(find_parameter_location(parameter_name), static_cast<i32>(texture_slot));
    }

    i32 material_t::find_parameter_location(const char* parameter_name)
    {
        const auto found = parameter_locations.find(parameter_name);

        if (found != parameter_locations.end())
        {
            return found->second;
        }

        const i32 location = shader.get_uniform_location(parameter_name);
        parameter_locations.emplace(parameter_name, location);

        return location;
    }
}

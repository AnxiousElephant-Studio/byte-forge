#include <byte-forge-engine/modules/rendering/lighting/point_light_renderer.h>
#include <byte-forge-engine/modules/rendering/opengl/state/opengl_render_state.h>

namespace byteforge::rendering
{
    namespace
    {
        const char* const vertex_source = R"(#version 460 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_uv;

uniform mat4 u_projection;
uniform mat4 u_model;

out vec2 v_world_position;

void main()
{
    const vec4 world_position = u_model * vec4(a_position, 1.0);

    v_world_position = world_position.xy;
    gl_Position      = u_projection * world_position;
}
)";

        const char* const fragment_source = R"(#version 460 core

in vec2 v_world_position;

uniform vec2  u_light_position;
uniform float u_light_radius;
uniform vec4  u_light_color;
uniform float u_light_intensity;

out vec4 out_color;

void main()
{
    const float distance_to_light = length(v_world_position - u_light_position);

    if (distance_to_light > u_light_radius)
    {
        out_color = vec4(0.0);
        return;
    }

    const float falloff = 1.0 - distance_to_light / u_light_radius;

    out_color = u_light_color * u_light_intensity * falloff;
}
)";

        const char* const shader_label = "<point light>";
    }

    bool point_light_renderer_t::initialize()
    {
        if (!shader.create_from_source(vertex_source, fragment_source, shader_label, shader_label))
        {
            return false;
        }

        if (!quad.create_quad())
        {
            shader.dispose();
            return false;
        }

        projection_location      = shader.get_uniform_location("u_projection");
        model_location           = shader.get_uniform_location("u_model");
        light_position_location  = shader.get_uniform_location("u_light_position");
        light_radius_location    = shader.get_uniform_location("u_light_radius");
        light_color_location     = shader.get_uniform_location("u_light_color");
        light_intensity_location = shader.get_uniform_location("u_light_intensity");

        return true;
    }

    void point_light_renderer_t::dispose()
    {
        quad.dispose();
        shader.dispose();
    }

    void point_light_renderer_t::begin(const math::mat4_t& projection) const
    {
        opengl_render_state_t::enable_blending();
        opengl_render_state_t::use_additive_blending();

        shader.bind();
        shader.set_matrix_4(projection_location, projection);
    }

    void point_light_renderer_t::draw(const point_light_t& light) const
    {
        const f32 diameter = light.get_diameter();

        const math::mat4_t model = math::mat4_t::identity()
            .translate(light.position.x, light.position.y, 0.0f)
            .scale(diameter, diameter, 1.0f);

        shader.set_matrix_4(model_location, model);
        shader.set_vector_2(light_position_location, light.position);
        shader.set_float(light_radius_location, light.radius);
        shader.set_vector_4(light_color_location, light.color);
        shader.set_float(light_intensity_location, light.intensity);

        quad.draw();
    }
}

#version 460 core

in vec2 v_uv;
in vec2 v_world_position;

uniform vec2  u_light_position;
uniform float u_light_radius;
uniform vec4  u_light_color;
uniform float u_light_intensity;

out vec4 out_color;

void main()
{
    float distance = length(v_world_position - u_light_position);
    
    if (distance > u_light_radius) {
        out_color = vec4(0.0f);
        return;                
    }

    float t = distance / u_light_radius;
    t = 1 - t;
    out_color = u_light_color * u_light_intensity * t;
}

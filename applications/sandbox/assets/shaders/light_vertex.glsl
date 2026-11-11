#version 460 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_uv;

uniform mat4 u_projection;
uniform mat4 u_model;

out vec2 v_uv;
out vec2 v_world_position;

void main()
{
    const vec4 world_position = u_model * vec4(a_position, 1.0);

    v_uv             = a_uv;
    v_world_position = world_position.xy;
    gl_Position      = u_projection * world_position;
}

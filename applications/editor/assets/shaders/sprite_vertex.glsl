#version 460 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_texture_coordinates;

uniform mat4 u_projection_matrix;
uniform mat4 u_model_matrix;
uniform vec4 u_uv_rect;

out vec2 vertex_texture_coordinates;

void main()
{
    vertex_texture_coordinates = u_uv_rect.xy + in_texture_coordinates * u_uv_rect.zw;
    gl_Position = u_projection_matrix * u_model_matrix * vec4(in_position, 1.0);
}

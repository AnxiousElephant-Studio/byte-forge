#version 460 core

in vec2 vertex_texture_coordinates;

uniform sampler2D u_texture;
uniform vec4 u_tint;
uniform bool u_use_texture;

out vec4 fragment_color;

void main()
{
    fragment_color = u_use_texture ? texture(u_texture, vertex_texture_coordinates) * u_tint : u_tint;
}

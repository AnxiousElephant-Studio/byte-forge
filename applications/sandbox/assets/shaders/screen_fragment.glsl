#version 460 core

in vec2 v_uv;

uniform sampler2D u_scene;
uniform sampler2D u_light;

out vec4 out_color;

void main()
{
    float ambientStrength = 0.9;

    out_color = texture(u_scene, v_uv) * (texture(u_light, v_uv) * ambientStrength);
}

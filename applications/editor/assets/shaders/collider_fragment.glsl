#version 460 core

out vec4 fragment_color;

void main()
{
    fragment_color = gl_FragCoord.y < 0.2f ? vec4(0.0f, 0.0f, 1.0f, 1.0f) : vec4(0.0f, 0.0f, 1.0f, 0.5f);
}

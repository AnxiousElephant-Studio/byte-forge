#pragma once

namespace byteforge::rendering
{
    inline constexpr const char* sprite_material_template = R"(uniform sampler2D u_texture;
uniform bool u_use_texture;
uniform vec4 u_tint;

void surface(in material_input_t surface_input, inout material_output_t surface_output)
{
    const vec4 color = u_use_texture ? texture(u_texture, surface_input.uv) * u_tint : u_tint;

    surface_output.base_color = color.rgb;
    surface_output.opacity    = color.a;
}
)";
}

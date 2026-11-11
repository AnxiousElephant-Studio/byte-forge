uniform vec4  u_line_color;
uniform float u_pan_speed;

const float line_pixels = 2.0;

const float stripe_size = 0.1;
const float space_size  = 0.1;
const float period_size = stripe_size + space_size;

const float space_opacity = 0.1;

void surface(in material_input_t surface_input, inout material_output_t surface_output)
{
    const vec2 uv = surface_input.uv;

    const vec2 line_uv = fwidth(uv) * line_pixels;

    surface_output.base_color = u_line_color.rgb;

    if (uv.x < line_uv.x || uv.x > 1.0 - line_uv.x || uv.y < line_uv.y || uv.y > 1.0 - line_uv.y)
    {
        surface_output.opacity = u_line_color.a;
        return;
    }

    const float period_position = fract((uv.y - surface_input.time * u_pan_speed) / period_size) * period_size;

    const bool is_stripe = period_position < stripe_size;

    if (!is_stripe)
    {
        surface_output.opacity = space_opacity;
        return;
    }

    const bool is_outline = period_position < line_uv.y || period_position > stripe_size - line_uv.y;

    surface_output.opacity = is_outline ? u_line_color.a : 0.0;
}

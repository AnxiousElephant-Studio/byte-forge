#pragma once

#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/math/vectors/vector2.h>
#include <byte-forge-engine/modules/math/vectors/vector4.h>

namespace byteforge::rendering
{
    struct point_light_t
    {
    public:
        static constexpr f32 default_radius    = 250.0f;
        static constexpr f32 default_intensity = 1.0f;

        math::vec2_t position;
        f32 radius;
        math::vec4_t color;
        f32 intensity;

        point_light_t()
            : position()
            , radius(default_radius)
            , color(1.0f, 0.85f, 0.6f, 1.0f)
            , intensity(default_intensity)
        {
        }

        f32 get_diameter() const { return radius * 2.0f; }
    };
}

#pragma once

#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/math/vectors/vector4.h>

namespace byteforge::components
{
    struct point_light_2D_component_t
    {
        math::vec4_t color;
        f32 radius;
        f32 intensity;
    };
}

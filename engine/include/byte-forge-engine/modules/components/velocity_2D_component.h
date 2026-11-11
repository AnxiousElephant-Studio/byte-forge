#pragma once

#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/math/vectors/vector2.h>

namespace byteforge::components
{
    struct velocity_2D_component_t
    {
        math::vec2_t velocity;
        f32 speed;
    };
}

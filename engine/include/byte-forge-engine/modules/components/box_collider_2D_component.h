#pragma once

#include <byte-forge-engine/modules/math/vectors/vector2.h>

namespace byteforge::components
{
    struct box_collider_2D_component_t
    {
        math::vec2_t offset;
        math::vec2_t size;
    };
}

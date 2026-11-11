#pragma once

#include <byte-forge-engine/api/types.h>

namespace byteforge::math
{
    struct math_constants_t
    {
    public:
        constexpr static f32 PI      = 3.14159265358979323846f;
        constexpr static f32 HALF_PI = PI / 2.0f;
        constexpr static f32 TAU     = PI * 2.0f;
        constexpr static f32 HALF    = 0.5f;
    };
}

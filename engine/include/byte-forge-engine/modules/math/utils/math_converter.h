#pragma once

#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/math/utils/math_constants.h>

namespace byteforge::math
{
    struct math_converter_t
    {
    public:
        constexpr static f32 deg_to_rad(f32 degrees)
        {
            return degrees * (math_constants_t::PI / 180.0f);
        }

        constexpr static f32 rad_to_deg(f32 radians)
        {
            return radians * (180.0f / math_constants_t::PI);
        }
    };
}

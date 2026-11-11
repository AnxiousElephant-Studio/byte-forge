#pragma once

#include <byte-forge-engine/api/types.h>

namespace byteforge::components
{
    struct camera_2D_component_t
    {
        f32 zoom;
        bool is_primary;
    };
}

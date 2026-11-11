#pragma once

#include <byte-forge-engine/api/types.h>

namespace byteforge::platform
{
    enum class e_input_states : u8
    {
        Pressed   = 0,
        Released  = 1,
        Held      = 2
    };
}

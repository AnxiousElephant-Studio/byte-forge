#pragma once

#include <byte-forge-engine/api/types.h>

namespace byteforge::platform
{
    enum class e_input_event_types : u8
    {
        None,
        Key,
        MouseButton,
        MouseMove,
        MouseWheel
    };
}

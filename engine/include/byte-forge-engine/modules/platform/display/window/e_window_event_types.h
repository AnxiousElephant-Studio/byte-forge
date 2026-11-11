#pragma once

#include <byte-forge-engine/api/types.h>

namespace byteforge::platform
{
    enum class e_window_event_types : u8
    {
        None,
        Resize,
        CloseRequested
    };
}

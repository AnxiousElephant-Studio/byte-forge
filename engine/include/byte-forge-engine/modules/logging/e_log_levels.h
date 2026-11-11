#pragma once

#include <byte-forge-engine/api/types.h>

namespace byteforge::logging
{
    enum class e_log_levels : u8
    {
        Info    = 0,
        Debug   = 1,
        Trace   = 2,
        Success = 3,
        Warning = 4,
        Error   = 5,
        Fatal   = 6
    };
}

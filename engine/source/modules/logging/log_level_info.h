#pragma once

#include <byte-forge-engine/modules/logging/e_log_levels.h>

namespace byteforge::logging
{
    struct log_level_info_t
    {
        static const char* get_color(e_log_levels level);
        static const char* get_name(e_log_levels level);
    };
}

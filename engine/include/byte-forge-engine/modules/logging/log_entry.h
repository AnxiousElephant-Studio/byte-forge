#pragma once

#include <byte-forge-engine/modules/logging/e_log_levels.h>

#include <string>

namespace byteforge::logging
{
    struct log_entry_t
    {
    public:
        e_log_levels level;
        std::string line;
    };
}

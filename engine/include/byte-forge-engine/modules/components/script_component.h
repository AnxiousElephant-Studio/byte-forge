#pragma once

#include <byte-forge-engine/api/types.h>

namespace byteforge::components
{
    constexpr usize SCRIPT_PATH_MAX_LENGTH = 256;

    struct script_component_t
    {
        char script_path[SCRIPT_PATH_MAX_LENGTH];
    };
}

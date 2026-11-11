#pragma once

#include <byte-forge-engine/api/types.h>

namespace byteforge::components
{
    constexpr usize NAME_MAX_LENGTH = 64;

    struct name_component_t
    {
        char name[NAME_MAX_LENGTH];
    };
}

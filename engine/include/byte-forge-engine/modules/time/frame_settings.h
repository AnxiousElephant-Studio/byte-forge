#pragma once

#include <byte-forge-engine/api/types.h>

namespace byteforge::time
{
    struct frame_settings_t
    {
    public:
        u8 target_fps;

        explicit frame_settings_t(u8 target_fps) : target_fps(target_fps) {}
    };
}

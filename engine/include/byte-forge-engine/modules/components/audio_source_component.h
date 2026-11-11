#pragma once

#include <byte-forge-engine/api/types.h>

namespace byteforge::components
{
    constexpr usize AUDIO_CLIP_PATH_MAX_LENGTH = 256;

    struct audio_source_component_t
    {
        char clip_path[AUDIO_CLIP_PATH_MAX_LENGTH];
        f32 volume;
        bool loop;
        bool play_on_start;
    };
}

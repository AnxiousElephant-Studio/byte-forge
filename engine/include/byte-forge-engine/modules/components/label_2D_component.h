#pragma once

#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/math/vectors/vector4.h>

namespace byteforge::components
{
    constexpr usize LABEL_TEXT_MAX_LENGTH = 256;
    constexpr usize FONT_PATH_MAX_LENGTH  = 256;

    struct label_2D_component_t
    {
        char text[LABEL_TEXT_MAX_LENGTH];
        char font_path[FONT_PATH_MAX_LENGTH];
        f32 font_size;
        math::vec4_t color;
    };
}

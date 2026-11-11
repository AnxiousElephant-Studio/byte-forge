#pragma once

#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/math/vectors/vector2.h>
#include <byte-forge-engine/modules/math/vectors/vector4.h>

namespace byteforge::components
{
    constexpr usize TEXTURE_PATH_MAX_LENGTH  = 256;
    constexpr usize MATERIAL_PATH_MAX_LENGTH = 256;

    struct sprite_2D_component_t
    {
        char texture_path[TEXTURE_PATH_MAX_LENGTH];
        math::vec2_t size;
        math::vec4_t tint;
        char material_path[MATERIAL_PATH_MAX_LENGTH];
        bool is_lit;
    };
}

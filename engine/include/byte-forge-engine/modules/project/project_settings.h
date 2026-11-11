#pragma once

#include <byte-forge-engine/api/types.h>

#include <string>

namespace byteforge::project
{
    struct project_settings_t
    {
    public:
        u32 target_fps                     = 60;
        bool use_vsync                     = true;
        std::string assets_directory       = "assets";
        std::string levels_directory       = "levels";
        std::string sprite_vertex_shader   = "assets/shaders/sprite_vertex.glsl";
        std::string sprite_fragment_shader = "assets/shaders/sprite_fragment.glsl";
    };
}

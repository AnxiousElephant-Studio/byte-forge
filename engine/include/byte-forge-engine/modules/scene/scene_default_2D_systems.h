#pragma once

#include <byte-forge-engine/modules/scene/scene_api.h>
#include <byte-forge-engine/api/types.h>

namespace byteforge::scene
{
    class scene_context_t;

    struct scene_default_2D_system_orders_t
    {
    public:
        static constexpr i32 audio_start  = 100;
        static constexpr i32 script_start = 200;

        static constexpr i32 physics_2D   = 100;
        static constexpr i32 collision_2D = 200;
        static constexpr i32 script       = 300;

        static constexpr i32 render_2D_begin = 0;
        static constexpr i32 camera_2D       = 100;
        static constexpr i32 light_2D        = 150;
        static constexpr i32 sprite_2D       = 200;
        static constexpr i32 label_2D        = 300;

        static constexpr i32 script_stop = 100;
    };

    BF_SCENE_API bool add_default_2D_systems(scene_context_t& scene_context);
    BF_SCENE_API void remove_default_2D_systems(scene_context_t& scene_context);
    BF_SCENE_API bool has_any_default_2D_system(const scene_context_t& scene_context);
}

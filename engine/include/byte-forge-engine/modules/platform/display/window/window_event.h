#pragma once

#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/platform/display/window/e_window_event_types.h>
#include <byte-forge-engine/modules/math/vectors/vector2i.h>

namespace byteforge::platform
{
    struct window_event_t
    {
    public:
        e_window_event_types type;
        u32                  window_id;

        union
        {
            struct
            {
                math::vec2i_t new_size;
            } resize;
        };

        window_event_t() : type(e_window_event_types::None), window_id(0), resize() {}
    };
}

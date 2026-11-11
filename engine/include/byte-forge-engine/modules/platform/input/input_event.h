#pragma once

#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/platform/input/e_input_event_types.h>
#include <byte-forge-engine/modules/platform/input/e_input_states.h>
#include <byte-forge-engine/modules/platform/input/e_key_codes.h>
#include <byte-forge-engine/modules/platform/input/e_mouse_buttons.h>

namespace byteforge::platform
{
    struct input_event_t
    {
    public:
        e_input_event_types type;
        u32                 window_id;

        union
        {
            struct
            {
                e_key_codes     code;
                e_input_states  state;
            } key;

            struct
            {
                e_mouse_buttons     button;
                e_input_states      state;
                f32                 x, y;
            } mouse_button;

            struct
            {
                f32 x, y;
                f32 delta_x, delta_y;
            } mouse_move;

            struct
            {
                f32 delta_x, delta_y;
            } mouse_wheel;
        };

        input_event_t() : type(e_input_event_types::None), window_id(0), key() {}
    };
}

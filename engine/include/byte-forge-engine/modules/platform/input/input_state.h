#pragma once

#include <byte-forge-engine/modules/platform/platform_api.h>
#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/math/vectors/vector2.h>
#include <byte-forge-engine/modules/platform/input/e_key_codes.h>
#include <byte-forge-engine/modules/platform/input/e_mouse_buttons.h>
#include <byte-forge-engine/modules/platform/input/input_event.h>

namespace byteforge::platform
{
    struct BF_PLATFORM_API input_state_t
    {
    public:
        bool enabled;
        bool keys_down[KEYCODES_TABLE_SIZE];
        bool keys_pressed[KEYCODES_TABLE_SIZE];
        bool keys_released[KEYCODES_TABLE_SIZE];
        bool mouse_buttons_down[MOUSE_BUTTONS_TABLE_SIZE];
        bool mouse_buttons_pressed[MOUSE_BUTTONS_TABLE_SIZE];
        bool mouse_buttons_released[MOUSE_BUTTONS_TABLE_SIZE];
        math::vec2_t mouse_window_position;
        math::vec2_t mouse_position;
        math::vec2_t mouse_wheel;

        input_state_t();

        void begin_frame();
        void process(const input_event_t& event);
        void clear();

        void set_mouse_target(const math::vec2_t& window_origin, const math::vec2_t& scale);

        bool is_key_down(const e_key_codes key_code) const;
        bool is_key_pressed(const e_key_codes key_code) const;
        bool is_key_released(const e_key_codes key_code) const;

        bool is_mouse_button_down(const e_mouse_buttons mouse_button) const;
        bool is_mouse_button_pressed(const e_mouse_buttons mouse_button) const;
        bool is_mouse_button_released(const e_mouse_buttons mouse_button) const;

    private:
        void update_mouse_position();

        math::vec2_t mouse_target_origin;
        math::vec2_t mouse_target_scale;
    };
}

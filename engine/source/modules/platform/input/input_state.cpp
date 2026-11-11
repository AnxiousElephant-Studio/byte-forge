#include <byte-forge-engine/modules/platform/input/input_state.h>

#include <cstring>

namespace byteforge::platform
{
    namespace
    {
        usize to_index(const e_key_codes key_code)
        {
            return static_cast<usize>(key_code);
        }

        usize to_index(const e_mouse_buttons mouse_button)
        {
            return static_cast<usize>(mouse_button);
        }
    }

    input_state_t::input_state_t()
        : enabled(true)
        , mouse_window_position()
        , mouse_position()
        , mouse_wheel()
        , mouse_target_origin()
        , mouse_target_scale(1.0f, 1.0f)
    {
        clear();
    }

    void input_state_t::begin_frame()
    {
        std::memset(keys_pressed, 0, sizeof(keys_pressed));
        std::memset(keys_released, 0, sizeof(keys_released));
        std::memset(mouse_buttons_pressed, 0, sizeof(mouse_buttons_pressed));
        std::memset(mouse_buttons_released, 0, sizeof(mouse_buttons_released));

        mouse_wheel = math::vec2_t::zero();
    }

    void input_state_t::process(const input_event_t& event)
    {
        if (event.type == e_input_event_types::MouseMove)
        {
            mouse_window_position = math::vec2_t(event.mouse_move.x, event.mouse_move.y);
            update_mouse_position();
        }

        if (!enabled)
        {
            return;
        }

        switch (event.type)
        {
            case e_input_event_types::Key:
            {
                const usize index = to_index(event.key.code);

                if (index >= KEYCODES_TABLE_SIZE)
                {
                    return;
                }

                if (event.key.state == e_input_states::Pressed)
                {
                    keys_down[index]    = true;
                    keys_pressed[index] = true;
                }
                else if (event.key.state == e_input_states::Released)
                {
                    keys_down[index]     = false;
                    keys_released[index] = true;
                }

                return;
            }

            case e_input_event_types::MouseButton:
            {
                const usize index = to_index(event.mouse_button.button);

                if (index >= MOUSE_BUTTONS_TABLE_SIZE)
                {
                    return;
                }

                if (event.mouse_button.state == e_input_states::Pressed)
                {
                    mouse_buttons_down[index]    = true;
                    mouse_buttons_pressed[index] = true;
                }
                else if (event.mouse_button.state == e_input_states::Released)
                {
                    mouse_buttons_down[index]     = false;
                    mouse_buttons_released[index] = true;
                }

                return;
            }

            case e_input_event_types::MouseWheel:
            {
                mouse_wheel += math::vec2_t(event.mouse_wheel.delta_x, event.mouse_wheel.delta_y);
                return;
            }

            default:
                return;
        }
    }

    void input_state_t::clear()
    {
        std::memset(keys_down, 0, sizeof(keys_down));
        std::memset(mouse_buttons_down, 0, sizeof(mouse_buttons_down));

        begin_frame();
    }

    void input_state_t::set_mouse_target(const math::vec2_t& window_origin, const math::vec2_t& scale)
    {
        mouse_target_origin = window_origin;
        mouse_target_scale  = scale;

        update_mouse_position();
    }

    void input_state_t::update_mouse_position()
    {
        mouse_position = math::vec2_t(
            (mouse_window_position.x - mouse_target_origin.x) * mouse_target_scale.x,
            (mouse_window_position.y - mouse_target_origin.y) * mouse_target_scale.y);
    }

    bool input_state_t::is_key_down(const e_key_codes key_code) const
    {
        return to_index(key_code) < KEYCODES_TABLE_SIZE && keys_down[to_index(key_code)];
    }

    bool input_state_t::is_key_pressed(const e_key_codes key_code) const
    {
        return to_index(key_code) < KEYCODES_TABLE_SIZE && keys_pressed[to_index(key_code)];
    }

    bool input_state_t::is_key_released(const e_key_codes key_code) const
    {
        return to_index(key_code) < KEYCODES_TABLE_SIZE && keys_released[to_index(key_code)];
    }

    bool input_state_t::is_mouse_button_down(const e_mouse_buttons mouse_button) const
    {
        return to_index(mouse_button) < MOUSE_BUTTONS_TABLE_SIZE && mouse_buttons_down[to_index(mouse_button)];
    }

    bool input_state_t::is_mouse_button_pressed(const e_mouse_buttons mouse_button) const
    {
        return to_index(mouse_button) < MOUSE_BUTTONS_TABLE_SIZE && mouse_buttons_pressed[to_index(mouse_button)];
    }

    bool input_state_t::is_mouse_button_released(const e_mouse_buttons mouse_button) const
    {
        return to_index(mouse_button) < MOUSE_BUTTONS_TABLE_SIZE && mouse_buttons_released[to_index(mouse_button)];
    }
}

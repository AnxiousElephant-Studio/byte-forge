#include <byte-forge-engine/modules/platform/input/input_event_parser.h>

#include <byte-forge-engine/modules/platform/input/e_input_event_types.h>
#include <byte-forge-engine/modules/platform/input/e_input_states.h>
#include <byte-forge-engine/modules/platform/input/e_key_codes.h>
#include <byte-forge-engine/modules/platform/input/e_mouse_buttons.h>
#include <byte-forge-engine/modules/platform/input/input_event.h>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_scancode.h>

#include <array>

namespace byteforge::platform
{
    std::array<e_key_codes, KEYCODES_TABLE_SIZE> input_event_parser_t::create_keycodes_mapper()
    {
        std::array<e_key_codes, KEYCODES_TABLE_SIZE> mapper = {};
        mapper.fill(e_key_codes::Unknown);

        mapper[SDL_SCANCODE_BACKSPACE] = e_key_codes::Backspace;
        mapper[SDL_SCANCODE_TAB] = e_key_codes::Tab;
        mapper[SDL_SCANCODE_RETURN] = e_key_codes::Return;
        mapper[SDL_SCANCODE_ESCAPE] = e_key_codes::Escape;
        mapper[SDL_SCANCODE_SPACE] = e_key_codes::Space;
        mapper[SDL_SCANCODE_DELETE] = e_key_codes::Delete;

        mapper[SDL_SCANCODE_INSERT] = e_key_codes::Insert;
        mapper[SDL_SCANCODE_HOME] = e_key_codes::Home;
        mapper[SDL_SCANCODE_END] = e_key_codes::End;
        mapper[SDL_SCANCODE_PAGEUP] = e_key_codes::PageUp;
        mapper[SDL_SCANCODE_PAGEDOWN] = e_key_codes::PageDown;
        mapper[SDL_SCANCODE_LEFT] = e_key_codes::ArrowLeft;
        mapper[SDL_SCANCODE_RIGHT] = e_key_codes::ArrowRight;
        mapper[SDL_SCANCODE_UP] = e_key_codes::ArrowUp;
        mapper[SDL_SCANCODE_DOWN] = e_key_codes::ArrowDown;

        mapper[SDL_SCANCODE_CAPSLOCK] = e_key_codes::CapsLock;
        mapper[SDL_SCANCODE_NUMLOCKCLEAR] = e_key_codes::NumLock;
        mapper[SDL_SCANCODE_SCROLLLOCK] = e_key_codes::ScrollLock;
        mapper[SDL_SCANCODE_PRINTSCREEN] = e_key_codes::PrintScreen;
        mapper[SDL_SCANCODE_PAUSE] = e_key_codes::Pause;
        mapper[SDL_SCANCODE_LSHIFT] = e_key_codes::ShiftLeft;
        mapper[SDL_SCANCODE_RSHIFT] = e_key_codes::ShiftRight;
        mapper[SDL_SCANCODE_LCTRL] = e_key_codes::CtrlLeft;
        mapper[SDL_SCANCODE_RCTRL] = e_key_codes::CtrlRight;
        mapper[SDL_SCANCODE_LALT] = e_key_codes::AltLeft;
        mapper[SDL_SCANCODE_RALT] = e_key_codes::AltRight;
        mapper[SDL_SCANCODE_LGUI] = e_key_codes::SuperLeft;
        mapper[SDL_SCANCODE_RGUI] = e_key_codes::SuperRight;
        mapper[SDL_SCANCODE_MENU] = e_key_codes::Menu;

        mapper[SDL_SCANCODE_F1] = e_key_codes::F1;
        mapper[SDL_SCANCODE_F2] = e_key_codes::F2;
        mapper[SDL_SCANCODE_F3] = e_key_codes::F3;
        mapper[SDL_SCANCODE_F4] = e_key_codes::F4;
        mapper[SDL_SCANCODE_F5] = e_key_codes::F5;
        mapper[SDL_SCANCODE_F6] = e_key_codes::F6;
        mapper[SDL_SCANCODE_F7] = e_key_codes::F7;
        mapper[SDL_SCANCODE_F8] = e_key_codes::F8;
        mapper[SDL_SCANCODE_F9] = e_key_codes::F9;
        mapper[SDL_SCANCODE_F10] = e_key_codes::F10;
        mapper[SDL_SCANCODE_F11] = e_key_codes::F11;
        mapper[SDL_SCANCODE_F12] = e_key_codes::F12;
        mapper[SDL_SCANCODE_F13] = e_key_codes::F13;
        mapper[SDL_SCANCODE_F14] = e_key_codes::F14;
        mapper[SDL_SCANCODE_F15] = e_key_codes::F15;
        mapper[SDL_SCANCODE_F16] = e_key_codes::F16;
        mapper[SDL_SCANCODE_F17] = e_key_codes::F17;
        mapper[SDL_SCANCODE_F18] = e_key_codes::F18;
        mapper[SDL_SCANCODE_F19] = e_key_codes::F19;
        mapper[SDL_SCANCODE_F20] = e_key_codes::F20;
        mapper[SDL_SCANCODE_F21] = e_key_codes::F21;
        mapper[SDL_SCANCODE_F22] = e_key_codes::F22;
        mapper[SDL_SCANCODE_F23] = e_key_codes::F23;
        mapper[SDL_SCANCODE_F24] = e_key_codes::F24;

        mapper[SDL_SCANCODE_KP_0] = e_key_codes::Numpad0;
        mapper[SDL_SCANCODE_KP_1] = e_key_codes::Numpad1;
        mapper[SDL_SCANCODE_KP_2] = e_key_codes::Numpad2;
        mapper[SDL_SCANCODE_KP_3] = e_key_codes::Numpad3;
        mapper[SDL_SCANCODE_KP_4] = e_key_codes::Numpad4;
        mapper[SDL_SCANCODE_KP_5] = e_key_codes::Numpad5;
        mapper[SDL_SCANCODE_KP_6] = e_key_codes::Numpad6;
        mapper[SDL_SCANCODE_KP_7] = e_key_codes::Numpad7;
        mapper[SDL_SCANCODE_KP_8] = e_key_codes::Numpad8;
        mapper[SDL_SCANCODE_KP_9] = e_key_codes::Numpad9;
        mapper[SDL_SCANCODE_KP_PERIOD] = e_key_codes::NumpadDecimal;
        mapper[SDL_SCANCODE_KP_DIVIDE] = e_key_codes::NumpadDivide;
        mapper[SDL_SCANCODE_KP_MULTIPLY] = e_key_codes::NumpadMultiply;
        mapper[SDL_SCANCODE_KP_MINUS] = e_key_codes::NumpadSubtract;
        mapper[SDL_SCANCODE_KP_PLUS] = e_key_codes::NumpadAdd;
        mapper[SDL_SCANCODE_KP_ENTER] = e_key_codes::NumpadEnter;
        mapper[SDL_SCANCODE_KP_EQUALS] = e_key_codes::NumpadEqual;

        mapper[SDL_SCANCODE_A] = e_key_codes::A;
        mapper[SDL_SCANCODE_B] = e_key_codes::B;
        mapper[SDL_SCANCODE_C] = e_key_codes::C;
        mapper[SDL_SCANCODE_D] = e_key_codes::D;
        mapper[SDL_SCANCODE_E] = e_key_codes::E;
        mapper[SDL_SCANCODE_F] = e_key_codes::F;
        mapper[SDL_SCANCODE_G] = e_key_codes::G;
        mapper[SDL_SCANCODE_H] = e_key_codes::H;
        mapper[SDL_SCANCODE_I] = e_key_codes::I;
        mapper[SDL_SCANCODE_J] = e_key_codes::J;
        mapper[SDL_SCANCODE_K] = e_key_codes::K;
        mapper[SDL_SCANCODE_L] = e_key_codes::L;
        mapper[SDL_SCANCODE_M] = e_key_codes::M;
        mapper[SDL_SCANCODE_N] = e_key_codes::N;
        mapper[SDL_SCANCODE_O] = e_key_codes::O;
        mapper[SDL_SCANCODE_P] = e_key_codes::P;
        mapper[SDL_SCANCODE_Q] = e_key_codes::Q;
        mapper[SDL_SCANCODE_R] = e_key_codes::R;
        mapper[SDL_SCANCODE_S] = e_key_codes::S;
        mapper[SDL_SCANCODE_T] = e_key_codes::T;
        mapper[SDL_SCANCODE_U] = e_key_codes::U;
        mapper[SDL_SCANCODE_V] = e_key_codes::V;
        mapper[SDL_SCANCODE_W] = e_key_codes::W;
        mapper[SDL_SCANCODE_X] = e_key_codes::X;
        mapper[SDL_SCANCODE_Y] = e_key_codes::Y;
        mapper[SDL_SCANCODE_Z] = e_key_codes::Z;

        mapper[SDL_SCANCODE_0] = e_key_codes::Num0;
        mapper[SDL_SCANCODE_1] = e_key_codes::Num1;
        mapper[SDL_SCANCODE_2] = e_key_codes::Num2;
        mapper[SDL_SCANCODE_3] = e_key_codes::Num3;
        mapper[SDL_SCANCODE_4] = e_key_codes::Num4;
        mapper[SDL_SCANCODE_5] = e_key_codes::Num5;
        mapper[SDL_SCANCODE_6] = e_key_codes::Num6;
        mapper[SDL_SCANCODE_7] = e_key_codes::Num7;
        mapper[SDL_SCANCODE_8] = e_key_codes::Num8;
        mapper[SDL_SCANCODE_9] = e_key_codes::Num9;

        mapper[SDL_SCANCODE_MINUS] = e_key_codes::Minus;
        mapper[SDL_SCANCODE_EQUALS] = e_key_codes::Equal;
        mapper[SDL_SCANCODE_LEFTBRACKET] = e_key_codes::BracketLeft;
        mapper[SDL_SCANCODE_RIGHTBRACKET] = e_key_codes::BracketRight;
        mapper[SDL_SCANCODE_BACKSLASH] = e_key_codes::Backslash;
        mapper[SDL_SCANCODE_SEMICOLON] = e_key_codes::Semicolon;
        mapper[SDL_SCANCODE_APOSTROPHE] = e_key_codes::Apostrophe;
        mapper[SDL_SCANCODE_GRAVE] = e_key_codes::Grave;
        mapper[SDL_SCANCODE_COMMA] = e_key_codes::Comma;
        mapper[SDL_SCANCODE_PERIOD] = e_key_codes::Period;
        mapper[SDL_SCANCODE_SLASH] = e_key_codes::Slash;

        mapper[SDL_SCANCODE_MEDIA_PLAY] = e_key_codes::MediaPlay;
        mapper[SDL_SCANCODE_MEDIA_STOP] = e_key_codes::MediaStop;
        mapper[SDL_SCANCODE_MEDIA_NEXT_TRACK] = e_key_codes::MediaNext;
        mapper[SDL_SCANCODE_MEDIA_PREVIOUS_TRACK] = e_key_codes::MediaPrevious;
        mapper[SDL_SCANCODE_VOLUMEUP] = e_key_codes::MediaVolumeUp;
        mapper[SDL_SCANCODE_VOLUMEDOWN] = e_key_codes::MediaVolumeDown;
        mapper[SDL_SCANCODE_MUTE] = e_key_codes::MediaMute;

        return mapper;
    }

    const std::array<e_key_codes, KEYCODES_TABLE_SIZE> input_event_parser_t::keycodes_mapper = create_keycodes_mapper();

    const std::array<e_mouse_buttons, MOUSE_BUTTONS_TABLE_SIZE> input_event_parser_t::mouse_buttons_mapper =
    {
        e_mouse_buttons::None,
        e_mouse_buttons::Left,
        e_mouse_buttons::Right,
        e_mouse_buttons::Middle,
        e_mouse_buttons::WheelUp,
        e_mouse_buttons::WheelDown,
        e_mouse_buttons::WheelLeft,
        e_mouse_buttons::WheelRight,
        e_mouse_buttons::XButton1,
        e_mouse_buttons::XButton2,
    };

    bool input_event_parser_t::parse(const SDL_Event& sdl_event, input_event_t& event)
    {
        switch (sdl_event.type)
        {
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:
            {
                if (sdl_event.key.scancode >= keycodes_mapper.size())
                {
                    return false;
                }

                event.type = e_input_event_types::Key;
                event.window_id = sdl_event.key.windowID;
                event.key.code = keycodes_mapper[sdl_event.key.scancode];
                event.key.state = sdl_event.key.repeat ? e_input_states::Held : (sdl_event.type == SDL_EVENT_KEY_DOWN ? e_input_states::Pressed : e_input_states::Released);
                return true;
            }

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
            {
                if (sdl_event.button.button >= mouse_buttons_mapper.size())
                {
                    return false;
                }

                event.type = e_input_event_types::MouseButton;
                event.window_id = sdl_event.button.windowID;
                event.mouse_button.button = mouse_buttons_mapper[sdl_event.button.button];
                event.mouse_button.state = sdl_event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ? e_input_states::Pressed : e_input_states::Released;
                event.mouse_button.x = static_cast<f32>(sdl_event.button.x);
                event.mouse_button.y = static_cast<f32>(sdl_event.button.y);
                return true;
            }

            case SDL_EVENT_MOUSE_MOTION:
            {
                event.type = e_input_event_types::MouseMove;
                event.window_id = sdl_event.motion.windowID;
                event.mouse_move.x = static_cast<f32>(sdl_event.motion.x);
                event.mouse_move.y = static_cast<f32>(sdl_event.motion.y);
                event.mouse_move.delta_x = static_cast<f32>(sdl_event.motion.xrel);
                event.mouse_move.delta_y = static_cast<f32>(sdl_event.motion.yrel);
                return true;
            }

            case SDL_EVENT_MOUSE_WHEEL:
            {
                event.type = e_input_event_types::MouseWheel;
                event.window_id = sdl_event.wheel.windowID;
                event.mouse_wheel.delta_x = static_cast<f32>(sdl_event.wheel.x);
                event.mouse_wheel.delta_y = static_cast<f32>(sdl_event.wheel.y);
                return true;
            }
        }

        return false;
    }
}

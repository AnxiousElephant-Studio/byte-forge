#include <byte-forge-engine/modules/platform/input/input_names.h>
#include <byte-forge-engine/api/types.h>

#include <cstring>
#include <iterator>

namespace byteforge::platform
{
    namespace
    {
        struct key_name_t
        {
            const char* name;
            e_key_codes key_code;
        };

        struct mouse_button_name_t
        {
            const char* name;
            e_mouse_buttons mouse_button;
        };

        const key_name_t key_names[] =
        {
            { "Backspace",       e_key_codes::Backspace },
            { "Tab",             e_key_codes::Tab },
            { "Return",          e_key_codes::Return },
            { "Escape",          e_key_codes::Escape },
            { "Space",           e_key_codes::Space },
            { "Delete",          e_key_codes::Delete },
            { "Insert",          e_key_codes::Insert },
            { "Home",            e_key_codes::Home },
            { "End",             e_key_codes::End },
            { "PageUp",          e_key_codes::PageUp },
            { "PageDown",        e_key_codes::PageDown },
            { "ArrowLeft",       e_key_codes::ArrowLeft },
            { "ArrowRight",      e_key_codes::ArrowRight },
            { "ArrowUp",         e_key_codes::ArrowUp },
            { "ArrowDown",       e_key_codes::ArrowDown },
            { "CapsLock",        e_key_codes::CapsLock },
            { "NumLock",         e_key_codes::NumLock },
            { "ScrollLock",      e_key_codes::ScrollLock },
            { "PrintScreen",     e_key_codes::PrintScreen },
            { "Pause",           e_key_codes::Pause },
            { "ShiftLeft",       e_key_codes::ShiftLeft },
            { "ShiftRight",      e_key_codes::ShiftRight },
            { "CtrlLeft",        e_key_codes::CtrlLeft },
            { "CtrlRight",       e_key_codes::CtrlRight },
            { "AltLeft",         e_key_codes::AltLeft },
            { "AltRight",        e_key_codes::AltRight },
            { "SuperLeft",       e_key_codes::SuperLeft },
            { "SuperRight",      e_key_codes::SuperRight },
            { "Menu",            e_key_codes::Menu },
            { "F1",              e_key_codes::F1 },
            { "F2",              e_key_codes::F2 },
            { "F3",              e_key_codes::F3 },
            { "F4",              e_key_codes::F4 },
            { "F5",              e_key_codes::F5 },
            { "F6",              e_key_codes::F6 },
            { "F7",              e_key_codes::F7 },
            { "F8",              e_key_codes::F8 },
            { "F9",              e_key_codes::F9 },
            { "F10",             e_key_codes::F10 },
            { "F11",             e_key_codes::F11 },
            { "F12",             e_key_codes::F12 },
            { "F13",             e_key_codes::F13 },
            { "F14",             e_key_codes::F14 },
            { "F15",             e_key_codes::F15 },
            { "F16",             e_key_codes::F16 },
            { "F17",             e_key_codes::F17 },
            { "F18",             e_key_codes::F18 },
            { "F19",             e_key_codes::F19 },
            { "F20",             e_key_codes::F20 },
            { "F21",             e_key_codes::F21 },
            { "F22",             e_key_codes::F22 },
            { "F23",             e_key_codes::F23 },
            { "F24",             e_key_codes::F24 },
            { "Numpad0",         e_key_codes::Numpad0 },
            { "Numpad1",         e_key_codes::Numpad1 },
            { "Numpad2",         e_key_codes::Numpad2 },
            { "Numpad3",         e_key_codes::Numpad3 },
            { "Numpad4",         e_key_codes::Numpad4 },
            { "Numpad5",         e_key_codes::Numpad5 },
            { "Numpad6",         e_key_codes::Numpad6 },
            { "Numpad7",         e_key_codes::Numpad7 },
            { "Numpad8",         e_key_codes::Numpad8 },
            { "Numpad9",         e_key_codes::Numpad9 },
            { "NumpadDecimal",   e_key_codes::NumpadDecimal },
            { "NumpadDivide",    e_key_codes::NumpadDivide },
            { "NumpadMultiply",  e_key_codes::NumpadMultiply },
            { "NumpadSubtract",  e_key_codes::NumpadSubtract },
            { "NumpadAdd",       e_key_codes::NumpadAdd },
            { "NumpadEnter",     e_key_codes::NumpadEnter },
            { "NumpadEqual",     e_key_codes::NumpadEqual },
            { "A",               e_key_codes::A },
            { "B",               e_key_codes::B },
            { "C",               e_key_codes::C },
            { "D",               e_key_codes::D },
            { "E",               e_key_codes::E },
            { "F",               e_key_codes::F },
            { "G",               e_key_codes::G },
            { "H",               e_key_codes::H },
            { "I",               e_key_codes::I },
            { "J",               e_key_codes::J },
            { "K",               e_key_codes::K },
            { "L",               e_key_codes::L },
            { "M",               e_key_codes::M },
            { "N",               e_key_codes::N },
            { "O",               e_key_codes::O },
            { "P",               e_key_codes::P },
            { "Q",               e_key_codes::Q },
            { "R",               e_key_codes::R },
            { "S",               e_key_codes::S },
            { "T",               e_key_codes::T },
            { "U",               e_key_codes::U },
            { "V",               e_key_codes::V },
            { "W",               e_key_codes::W },
            { "X",               e_key_codes::X },
            { "Y",               e_key_codes::Y },
            { "Z",               e_key_codes::Z },
            { "Num0",            e_key_codes::Num0 },
            { "Num1",            e_key_codes::Num1 },
            { "Num2",            e_key_codes::Num2 },
            { "Num3",            e_key_codes::Num3 },
            { "Num4",            e_key_codes::Num4 },
            { "Num5",            e_key_codes::Num5 },
            { "Num6",            e_key_codes::Num6 },
            { "Num7",            e_key_codes::Num7 },
            { "Num8",            e_key_codes::Num8 },
            { "Num9",            e_key_codes::Num9 },
            { "Minus",           e_key_codes::Minus },
            { "Equal",           e_key_codes::Equal },
            { "BracketLeft",     e_key_codes::BracketLeft },
            { "BracketRight",    e_key_codes::BracketRight },
            { "Backslash",       e_key_codes::Backslash },
            { "Semicolon",       e_key_codes::Semicolon },
            { "Apostrophe",      e_key_codes::Apostrophe },
            { "Grave",           e_key_codes::Grave },
            { "Comma",           e_key_codes::Comma },
            { "Period",          e_key_codes::Period },
            { "Slash",           e_key_codes::Slash },
            { "MediaPlay",       e_key_codes::MediaPlay },
            { "MediaStop",       e_key_codes::MediaStop },
            { "MediaNext",       e_key_codes::MediaNext },
            { "MediaPrevious",   e_key_codes::MediaPrevious },
            { "MediaVolumeUp",   e_key_codes::MediaVolumeUp },
            { "MediaVolumeDown", e_key_codes::MediaVolumeDown },
            { "MediaMute",       e_key_codes::MediaMute },
        };

        const mouse_button_name_t mouse_button_names[] =
        {
            { "Left",     e_mouse_buttons::Left     },
            { "Right",    e_mouse_buttons::Right    },
            { "Middle",   e_mouse_buttons::Middle   },
            { "XButton1", e_mouse_buttons::XButton1 },
            { "XButton2", e_mouse_buttons::XButton2 },
        };
    }

    bool input_names_t::get_key_code(const char* name, e_key_codes& out_key_code)
    {
        for (usize index = 0; index < std::size(key_names); index++)
        {
            if (std::strcmp(key_names[index].name, name) == 0)
            {
                out_key_code = key_names[index].key_code;
                return true;
            }
        }

        return false;
    }

    bool input_names_t::get_mouse_button(const char* name, e_mouse_buttons& out_mouse_button)
    {
        for (usize index = 0; index < std::size(mouse_button_names); index++)
        {
            if (std::strcmp(mouse_button_names[index].name, name) == 0)
            {
                out_mouse_button = mouse_button_names[index].mouse_button;
                return true;
            }
        }

        return false;
    }
}

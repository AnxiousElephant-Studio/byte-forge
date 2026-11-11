#pragma once

#include <byte-forge-engine/api/types.h>

namespace byteforge::platform
{
    constexpr u32 KEYCODES_TABLE_SIZE = 512;

    enum class e_key_codes : u16
    {
        Unknown     = 0,

        Backspace   = 8,
        Tab         = 9,
        Return      = 13,
        Escape      = 27,
        Space       = 32,
        Delete      = 127,

        Insert      = 256,
        Home,
        End,
        PageUp,
        PageDown,
        ArrowLeft,
        ArrowRight,
        ArrowUp,
        ArrowDown,

        CapsLock,
        NumLock,
        ScrollLock,
        PrintScreen,
        Pause,
        ShiftLeft,
        ShiftRight,
        CtrlLeft,
        CtrlRight,
        AltLeft,
        AltRight,
        SuperLeft,
        SuperRight,
        Menu,

        F1,  F2,  F3,  F4,
        F5,  F6,  F7,  F8,
        F9,  F10, F11, F12,
        F13, F14, F15, F16,
        F17, F18, F19, F20,
        F21, F22, F23, F24,

        Numpad0, Numpad1, Numpad2, Numpad3, Numpad4,
        Numpad5, Numpad6, Numpad7, Numpad8, Numpad9,
        NumpadDecimal,
        NumpadDivide,
        NumpadMultiply,
        NumpadSubtract,
        NumpadAdd,
        NumpadEnter,
        NumpadEqual,

        A, B, C, D, E, F, G, H, I, J, K, L, M,
        N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

        Num0, Num1, Num2, Num3, Num4,
        Num5, Num6, Num7, Num8, Num9,

        Minus,
        Equal,
        BracketLeft,
        BracketRight,
        Backslash,
        Semicolon,
        Apostrophe,
        Grave,
        Comma,
        Period,
        Slash,

        MediaPlay,
        MediaStop,
        MediaNext,
        MediaPrevious,
        MediaVolumeUp,
        MediaVolumeDown,
        MediaMute,
    };
}

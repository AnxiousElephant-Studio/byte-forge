#pragma once

#include <byte-forge-engine/api/types.h>

namespace byteforge::platform
{
    constexpr u8 MOUSE_BUTTONS_TABLE_SIZE = 10;

    enum class e_mouse_buttons : u8
    {
        None        = 0,
        Left        = 1,
        Right       = 2,
        Middle      = 3,
        WheelUp     = 4,
        WheelDown   = 5,
        WheelLeft   = 6,
        WheelRight  = 7,
        XButton1    = 8,
        XButton2    = 9,
    };
}

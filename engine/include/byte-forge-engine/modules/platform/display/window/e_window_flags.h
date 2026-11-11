#pragma once

#include <byte-forge-engine/api/types.h>

namespace byteforge::platform
{
    enum class e_window_flags : u8
    {
        None        =  0,
        Resizable   =  (1 << 0),
        Borderless  =  (1 << 1),
        Fullscreen  =  (1 << 2),
        Hidden      =  (1 << 3),
        Maximized   =  (1 << 4),
        Minimized   =  (1 << 5),
    };

    constexpr e_window_flags operator|(e_window_flags left_flag, e_window_flags right_flag)
    {
        return static_cast<e_window_flags>(static_cast<u8>(left_flag) | static_cast<u8>(right_flag));
    }

    constexpr e_window_flags operator&(e_window_flags left_flag, e_window_flags right_flag)
    {
        return static_cast<e_window_flags>(static_cast<u32>(left_flag) & static_cast<u32>(right_flag));
    }

    constexpr e_window_flags& operator|=(e_window_flags& left_flag, e_window_flags right_flag)
    {
        left_flag = left_flag | right_flag;
        return left_flag;
    }

    constexpr bool has_flag(e_window_flags flags, e_window_flags flag)
    {
        return (flags & flag) == flag;
    }
}

#pragma once

namespace byteforge::logging
{
    struct log_style_t
    {
        static constexpr const char* reset          = "\033[0m";
        static constexpr const char* white_text     = "\033[97m";
        static constexpr const char* red_text       = "\033[31m";
        static constexpr const char* yellow_text    = "\033[33m";
        static constexpr const char* green_text     = "\033[32m";
        static constexpr const char* blue_text      = "\033[34m";
        static constexpr const char* cyan_text      = "\033[36m";
        static constexpr const char* red_background = "\033[41m";
    };
}

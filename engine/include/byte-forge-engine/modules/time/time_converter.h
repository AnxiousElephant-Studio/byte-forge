#pragma once

#include <byte-forge-engine/api/types.h>

namespace byteforge::time
{
    struct time_converter_t
    {
    public:
        constexpr static f64 MS_PER_SEC = 1000.0;

        constexpr static f64 sec_to_ms(const f64 secs)
        {
            return secs * MS_PER_SEC;
        }

        constexpr static f64 ms_to_sec(const f64 ms)
        {
            return ms / MS_PER_SEC;
        }
    };
}

#pragma once

#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/platform/display/window/e_window_flags.h>
#include <byte-forge-engine/modules/math/rects/recti.h>

#include <cstdio>

namespace byteforge::platform
{
    constexpr usize WINDOW_TITLE_MAX_LENGTH = 256;

    struct window_descriptor_t
    {
    public:
        char title[WINDOW_TITLE_MAX_LENGTH];
        math::recti_t bounds;
        e_window_flags flags;
        bool use_vsync;

        window_descriptor_t(
            const char* window_title,
            const math::recti_t& window_bounds,
            const e_window_flags window_flags,
            bool use_vsync
        )
            : bounds(window_bounds),
            flags(window_flags),
            use_vsync(use_vsync)
        {
            std::snprintf(title, WINDOW_TITLE_MAX_LENGTH, "%s", window_title);
        }
    };
}

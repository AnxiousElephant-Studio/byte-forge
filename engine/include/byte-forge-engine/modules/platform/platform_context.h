#pragma once

#include <byte-forge-engine/modules/platform/display/window/window.h>
#include <byte-forge-engine/modules/platform/input/input_state.h>

namespace byteforge::platform
{
    struct platform_context_t
    {
    public:
        window_t* current_window;
        input_state_t input_state;

        platform_context_t() : current_window(nullptr), input_state() {}
    };
}

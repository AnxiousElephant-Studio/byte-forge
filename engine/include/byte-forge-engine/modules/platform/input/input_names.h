#pragma once

#include <byte-forge-engine/modules/platform/platform_api.h>
#include <byte-forge-engine/modules/platform/input/e_key_codes.h>
#include <byte-forge-engine/modules/platform/input/e_mouse_buttons.h>

namespace byteforge::platform
{
    struct BF_PLATFORM_API input_names_t
    {
    public:
        static bool get_key_code(const char* name, e_key_codes& out_key_code);
        static bool get_mouse_button(const char* name, e_mouse_buttons& out_mouse_button);
    };
}

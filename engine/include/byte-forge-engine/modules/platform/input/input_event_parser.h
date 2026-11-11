#pragma once

#include <byte-forge-engine/modules/platform/platform_api.h>
#include <byte-forge-engine/modules/platform/input/e_key_codes.h>
#include <byte-forge-engine/modules/platform/input/e_mouse_buttons.h>
#include <byte-forge-engine/modules/platform/input/input_event.h>

#include <array>

union SDL_Event;

namespace byteforge::platform
{
    class BF_PLATFORM_API input_event_parser_t
    {
    public:
        static const std::array<e_key_codes, KEYCODES_TABLE_SIZE> keycodes_mapper;
        static const std::array<e_mouse_buttons, MOUSE_BUTTONS_TABLE_SIZE> mouse_buttons_mapper;

        static bool parse(const SDL_Event& sdl_event, input_event_t& event);

    private:
        static std::array<e_key_codes, KEYCODES_TABLE_SIZE> create_keycodes_mapper();
    };
}

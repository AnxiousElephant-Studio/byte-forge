#pragma once

#include <byte-forge-engine/modules/platform/platform_api.h>
#include <byte-forge-engine/modules/platform/display/window/window_event.h>

union SDL_Event;

namespace byteforge::platform
{
    class BF_PLATFORM_API window_event_parser_t
    {
    public:
        static bool parse(const SDL_Event& sdl_event, window_event_t& event);
    };
}

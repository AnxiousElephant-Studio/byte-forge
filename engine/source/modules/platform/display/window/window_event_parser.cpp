#include <byte-forge-engine/modules/platform/display/window/window_event_parser.h>
#include <byte-forge-engine/modules/platform/display/window/e_window_event_types.h>
#include <byte-forge-engine/modules/math/vectors/vector2i.h>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>

namespace byteforge::platform
{
    bool window_event_parser_t::parse(const SDL_Event& sdl_event, window_event_t& event)
    {
        event.window_id = sdl_event.window.windowID;

        switch (sdl_event.type)
        {
            case SDL_EVENT_WINDOW_RESIZED:
            {
                event.type = e_window_event_types::Resize;

                math::vec2i_t new_size;
                SDL_GetWindowSizeInPixels(SDL_GetWindowFromID(event.window_id), &new_size.x, &new_size.y);
                event.resize.new_size = new_size;

                return true;
            }

            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            {
                event.type = e_window_event_types::CloseRequested;

                return true;
            }

            default:
                return false;
        }

        return false;
    }
}

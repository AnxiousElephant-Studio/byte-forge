#include <byte-forge-engine/modules/platform/platform.h>
#include <byte-forge-engine/modules/platform/input/input_event_parser.h>
#include <byte-forge-engine/modules/platform/display/window/window_event_parser.h>
#include <byte-forge-engine/modules/logging/log_macros.h>

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_events.h>

namespace byteforge::platform
{
    namespace
    {
        const char* const initialization_error_format = "Could not initialize the platform (SDL). Cause: %s";
    }

    bool platform_t::initialize()
    {
        if (!SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
        {
            LOG_FATAL(initialization_error_format, SDL_GetError());
            return false;
        }

        initialized = true;

        return true;
    }

    void platform_t::set_current_window(window_t& window)
    {
        context.current_window = &window;
    }

    window_t* platform_t::get_current_window() const
    {
        return context.current_window;
    }

    void platform_t::request_close()
    {
        if (should_close_handler)
        {
            *should_close_handler = true;
        }
    }

    void platform_t::poll_events(const input_event_callback_t& input_event_callback, const window_event_callback_t& window_event_callback)
    {
        SDL_Event sdl_event;

        while (SDL_PollEvent(&sdl_event))
        {
            native_event_callback(&sdl_event);

            if (sdl_event.type == SDL_EVENT_QUIT)
            {
                request_close();

                return;
            }

            window_event_t window_event = {};

            if (window_event_parser_t::parse(sdl_event, window_event))
            {
                window_event_callback(window_event);
            }

            input_event_t input_event = {};

            if (input_event_parser_t::parse(sdl_event, input_event))
            {
                input_event_callback(input_event);
            }
        }
    }

    void platform_t::dispose()
    {
        window_manager.dispose();

        if (!initialized)
        {
            return;
        }

        SDL_QuitSubSystem(SDL_INIT_EVENTS);
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        SDL_Quit();

        initialized = false;
    }
}

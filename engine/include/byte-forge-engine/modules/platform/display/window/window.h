#pragma once

#include <byte-forge-engine/modules/platform/platform_api.h>
#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/platform/display/window/window_descriptor.h>
#include <byte-forge-engine/modules/math/vectors/vector2i.h>

struct SDL_Window;

namespace byteforge::platform
{
    struct BF_PLATFORM_API window_t
    {
    public:
        SDL_Window* sdl_window;
        u32 sdl_window_id;
        window_descriptor_t descriptor;

        window_t(const window_descriptor_t& descriptor, SDL_Window* sdl_window, u32 sdl_window_id)
            : sdl_window(sdl_window)
            , sdl_window_id(sdl_window_id)
            , descriptor(descriptor)
        {
        }

        void set_title(const char* new_title) const;
        void set_resizable(bool is_resizable) const;
        void set_size(const math::vec2i_t& size) const;
        void center() const;
        void maximize() const;

        bool is_valid() const;
        math::vec2i_t get_size() const;

        void destroy();
    };
}

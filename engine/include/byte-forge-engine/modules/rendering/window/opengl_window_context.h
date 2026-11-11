#pragma once

#include <byte-forge-engine/modules/rendering/rendering_api.h>
#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/platform/display/window/window.h>
#include <byte-forge-engine/modules/rendering/opengl/functions/opengl_functions.h>

struct SDL_Window;
struct SDL_GLContextState;

namespace byteforge::rendering
{
    struct BF_RENDERING_API opengl_window_context_t
    {
    public:
        u32 window_id;
        SDL_Window* sdl_window;
        SDL_GLContextState* gl_context;

        opengl_window_context_t() : window_id(0), sdl_window(nullptr), gl_context(nullptr) {}

        static void set_context_attributes();

        static opengl_function_loader_t get_function_loader();

        bool create(const platform::window_t& window, const opengl_window_context_t* shared_context);
        void destroy();

        bool make_current() const;
        bool set_vsync(bool enabled) const;
        void swap_buffers() const;
    };
}

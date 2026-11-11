#include <byte-forge-engine/modules/rendering/window/opengl_window_context.h>

#include <SDL3/SDL_video.h>

namespace byteforge::rendering
{
    namespace
    {
        const i32 opengl_major_version = 4;
        const i32 opengl_minor_version = 6;
    }

    void opengl_window_context_t::set_context_attributes()
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, opengl_major_version);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, opengl_minor_version);
    }

    opengl_function_loader_t opengl_window_context_t::get_function_loader()
    {
        return reinterpret_cast<opengl_function_loader_t>(SDL_GL_GetProcAddress);
    }

    bool opengl_window_context_t::create(const platform::window_t& window, const opengl_window_context_t* shared_context)
    {
        window_id  = window.sdl_window_id;
        sdl_window = window.sdl_window;

        if (shared_context != nullptr)
        {
            shared_context->make_current();
            SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
        }

        gl_context = SDL_GL_CreateContext(sdl_window);

        SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 0);

        return gl_context != nullptr && make_current();
    }

    bool opengl_window_context_t::make_current() const
    {
        return gl_context != nullptr && SDL_GL_MakeCurrent(sdl_window, gl_context);
    }

    bool opengl_window_context_t::set_vsync(bool enabled) const
    {
        return make_current() && SDL_GL_SetSwapInterval(enabled ? 1 : 0);
    }

    void opengl_window_context_t::swap_buffers() const
    {
        SDL_GL_SwapWindow(sdl_window);
    }

    void opengl_window_context_t::destroy()
    {
        if (gl_context != nullptr)
        {
            SDL_GL_DestroyContext(gl_context);
            gl_context = nullptr;
        }

        sdl_window = nullptr;
        window_id  = 0;
    }
}

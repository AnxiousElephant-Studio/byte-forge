#include <byte-forge-engine/modules/platform/display/window/window.h>

#include <SDL3/SDL_video.h>

namespace byteforge::platform
{
    void window_t::set_title(const char* new_title) const
    {
        SDL_SetWindowTitle(sdl_window, new_title);
    }

    void window_t::set_resizable(bool is_resizable) const
    {
        SDL_SetWindowResizable(sdl_window, is_resizable);
    }

    void window_t::set_size(const math::vec2i_t& size) const
    {
        SDL_SetWindowSize(sdl_window, size.x, size.y);
    }

    void window_t::center() const
    {
        SDL_SetWindowPosition(sdl_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    }

    void window_t::maximize() const
    {
        SDL_MaximizeWindow(sdl_window);
    }

    bool window_t::is_valid() const
    {
        return sdl_window != nullptr && sdl_window_id != 0;
    }

    math::vec2i_t window_t::get_size() const
    {
        math::vec2i_t size;

        if (SDL_GetWindowSizeInPixels(sdl_window, &size.x, &size.y))
        {
            return size;
        }

        return math::vec2i_t::zero();
    }

    void window_t::destroy()
    {
        if (sdl_window != nullptr)
        {
            SDL_DestroyWindow(sdl_window);
            sdl_window = nullptr;
        }

        sdl_window_id = 0;
    }
}

#include <byte-forge-engine/modules/platform/display/window/window_manager.h>
#include <byte-forge-engine/modules/platform/display/window/e_window_flags.h>
#include <byte-forge-engine/modules/graphics/e_graphic_backends.h>
#include <byte-forge-engine/modules/math/vectors/vector2i.h>

#include <SDL3/SDL_video.h>

namespace byteforge::platform
{
    namespace
    {
        SDL_WindowFlags get_sdl_window_flags(const window_descriptor_t& descriptor, const display_descriptor_t& display_descriptor)
        {
            SDL_WindowFlags flags = SDL_WINDOW_HIGH_PIXEL_DENSITY;

            if (has_flag(descriptor.flags, e_window_flags::Resizable))
            {
                flags |= SDL_WINDOW_RESIZABLE;
            }

            if (has_flag(descriptor.flags, e_window_flags::Borderless))
            {
                flags |= SDL_WINDOW_BORDERLESS;
            }

            if (has_flag(descriptor.flags, e_window_flags::Fullscreen))
            {
                flags |= SDL_WINDOW_FULLSCREEN;
            }

            if (has_flag(descriptor.flags, e_window_flags::Hidden))
            {
                flags |= SDL_WINDOW_HIDDEN;
            }

            if (has_flag(descriptor.flags, e_window_flags::Maximized))
            {
                flags |= SDL_WINDOW_MAXIMIZED;
            }

            if (has_flag(descriptor.flags, e_window_flags::Minimized))
            {
                flags |= SDL_WINDOW_MINIMIZED;
            }

            if (display_descriptor.graphic_backend == graphics::e_graphic_backends::OpenGL)
            {
                flags |= SDL_WINDOW_OPENGL;
            }

            return flags;
        }
    }

    window_t* window_manager_t::create_window(const window_descriptor_t& descriptor, const display_descriptor_t& display_descriptor)
    {
        if (descriptor.bounds.is_empty())
        {
            return nullptr;
        }

        SDL_Window* sdl_window = SDL_CreateWindow(descriptor.title, descriptor.bounds.size.width(), descriptor.bounds.size.height(), get_sdl_window_flags(descriptor, display_descriptor));

        if (sdl_window == nullptr)
        {
            return nullptr;
        }

        if (descriptor.bounds.position != math::vec2i_t::zero())
        {
            SDL_SetWindowPosition(sdl_window, descriptor.bounds.position.x, descriptor.bounds.position.y);
        }

        window_t* window = new window_t(descriptor, sdl_window, SDL_GetWindowID(sdl_window));
        windows.push_back(window);

        return window;
    }

    window_t* window_manager_t::get_window(const u32 window_id)
    {
        for (usize index = 0; index < windows.size(); index++)
        {
            if (windows[index]->sdl_window_id == window_id)
            {
                return windows[index];
            }
        }

        return nullptr;
    }

    window_t* window_manager_t::get_window_at(const usize index)
    {
        return index < windows.size() ? windows[index] : nullptr;
    }

    usize window_manager_t::get_window_count() const
    {
        return windows.size();
    }

    void window_manager_t::destroy_window(const u32 window_id)
    {
        for (usize index = 0; index < windows.size(); index++)
        {
            if (windows[index]->sdl_window_id != window_id)
            {
                continue;
            }

            windows[index]->destroy();
            delete windows[index];

            windows.erase(windows.begin() + static_cast<isize>(index));

            return;
        }
    }

    void window_manager_t::dispose()
    {
        for (usize index = 0; index < windows.size(); index++)
        {
            windows[index]->destroy();
            delete windows[index];
        }

        windows.clear();
    }
}

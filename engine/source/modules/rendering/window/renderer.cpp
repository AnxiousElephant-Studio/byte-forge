#include <byte-forge-engine/modules/rendering/window/renderer.h>
#include <byte-forge-engine/modules/logging/log_macros.h>
#include <byte-forge-engine/modules/rendering/opengl/framebuffer/opengl_framebuffer.h>
#include <byte-forge-engine/modules/rendering/opengl/functions/opengl_functions.h>
#include <byte-forge-engine/modules/rendering/opengl/state/opengl_render_state.h>

namespace byteforge::rendering
{
    namespace
    {
        const char* const context_error_format   = "Could not create the OpenGL context for window ID: %u.";
        const char* const functions_error_format = "Could not load the OpenGL functions. Cause: the GLAD loader failed.";
        const char* const vsync_warning_format   = "Could not set vsync for window ID: %u.";

        const char* const unsupported_backend_error_format = "Could not initialize the renderer with graphic backend: '%s'. Cause: only the OpenGL backend is implemented.";
        const char* const not_initialized_error_format     = "Could not attach window ID: %u to the renderer. Cause: the renderer is not initialized.";
    }

    bool renderer_t::initialize(const graphics::e_graphic_backends requested_graphic_backend)
    {
        if (requested_graphic_backend != graphics::e_graphic_backends::OpenGL)
        {
            LOG_FATAL(unsupported_backend_error_format, graphics::get_graphic_backend_name(requested_graphic_backend));
            return false;
        }

        opengl_window_context_t::set_context_attributes();
        graphic_backend = requested_graphic_backend;

        return true;
    }

    bool renderer_t::attach_window(const platform::window_t& window)
    {
        if (!is_initialized())
        {
            LOG_ERROR(not_initialized_error_format, window.sdl_window_id);
            return false;
        }

        opengl_window_context_t window_context;

        if (!window_context.create(window, window_contexts.empty() ? nullptr : &window_contexts.front()))
        {
            LOG_ERROR(context_error_format, window.sdl_window_id);
            window_context.destroy();
            return false;
        }

        if (!opengl_functions_t::are_loaded() && !opengl_functions_t::load(opengl_window_context_t::get_function_loader()))
        {
            LOG_ERROR(functions_error_format);
            window_context.destroy();
            return false;
        }

        if (!window_context.set_vsync(window.descriptor.use_vsync))
        {
            LOG_WARNING(vsync_warning_format, window.sdl_window_id);
        }

        window_contexts.push_back(window_context);

        return true;
    }

    void renderer_t::detach_window(u32 window_id)
    {
        for (usize index = 0; index < window_contexts.size(); index++)
        {
            if (window_contexts[index].window_id != window_id)
            {
                continue;
            }

            window_contexts[index].destroy();
            window_contexts.erase(window_contexts.begin() + static_cast<isize>(index));

            return;
        }
    }

    bool renderer_t::set_vsync(const platform::window_t& window, bool enabled)
    {
        const opengl_window_context_t* window_context = find_window_context(window.sdl_window_id);

        if (window_context == nullptr || !window_context->set_vsync(enabled))
        {
            LOG_WARNING(vsync_warning_format, window.sdl_window_id);
            return false;
        }

        return true;
    }

    bool renderer_t::begin_frame(const platform::window_t& window)
    {
        opengl_window_context_t* window_context = find_window_context(window.sdl_window_id);

        if (window_context == nullptr || !window_context->make_current())
        {
            return false;
        }

        opengl_framebuffer_t::bind_window();
        set_viewport(math::recti_t::from_size(window.get_size()));
        clear(clear_color);

        return true;
    }

    void renderer_t::end_frame(const platform::window_t& window)
    {
        opengl_window_context_t* window_context = find_window_context(window.sdl_window_id);

        if (window_context != nullptr)
        {
            window_context->swap_buffers();
        }
    }

    void renderer_t::dispose()
    {
        for (usize index = 0; index < window_contexts.size(); index++)
        {
            window_contexts[index].destroy();
        }

        window_contexts.clear();
        graphic_backend = graphics::e_graphic_backends::None;
    }

    graphics::e_graphic_backends renderer_t::get_graphic_backend() const
    {
        return graphic_backend;
    }

    bool renderer_t::is_initialized() const
    {
        return graphic_backend != graphics::e_graphic_backends::None;
    }

    void renderer_t::clear(const math::vec4_t& color) const
    {
        opengl_render_state_t::clear_color(color);
    }

    void renderer_t::set_viewport(const math::recti_t& bounds) const
    {
        opengl_render_state_t::set_viewport(bounds);
    }

    opengl_window_context_t* renderer_t::find_window_context(u32 window_id)
    {
        for (usize index = 0; index < window_contexts.size(); index++)
        {
            if (window_contexts[index].window_id == window_id)
            {
                return &window_contexts[index];
            }
        }

        return nullptr;
    }
}

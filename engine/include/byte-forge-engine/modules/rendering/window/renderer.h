#pragma once

#include <byte-forge-engine/modules/rendering/rendering_api.h>
#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/graphics/e_graphic_backends.h>
#include <byte-forge-engine/modules/math/vectors/vector4.h>
#include <byte-forge-engine/modules/math/rects/recti.h>
#include <byte-forge-engine/modules/platform/display/window/window.h>
#include <byte-forge-engine/modules/rendering/window/opengl_window_context.h>

#include <vector>

namespace byteforge::rendering
{
    class BF_RENDERING_API renderer_t
    {
    public:
        math::vec4_t clear_color;

        renderer_t() : clear_color(0.0f, 0.0f, 0.0f, 1.0f), graphic_backend(graphics::e_graphic_backends::None), window_contexts() {}

        bool initialize(const graphics::e_graphic_backends requested_graphic_backend);
        void dispose();

        bool attach_window(const platform::window_t& window);
        void detach_window(u32 window_id);

        bool set_vsync(const platform::window_t& window, bool enabled);

        bool begin_frame(const platform::window_t& window);
        void end_frame(const platform::window_t& window);

        void clear(const math::vec4_t& color) const;
        void set_viewport(const math::recti_t& bounds) const;

        graphics::e_graphic_backends get_graphic_backend() const;
        bool is_initialized() const;

    private:
        opengl_window_context_t* find_window_context(u32 window_id);

        graphics::e_graphic_backends graphic_backend;
        std::vector<opengl_window_context_t> window_contexts;
    };
}

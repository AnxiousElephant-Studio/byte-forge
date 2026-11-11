#pragma once

#include <byte-forge-engine/modules/rendering/rendering_api.h>
#include <byte-forge-engine/modules/math/vectors/vector2i.h>
#include <byte-forge-engine/modules/math/vectors/vector4.h>
#include <byte-forge-engine/modules/rendering/opengl/framebuffer/opengl_framebuffer.h>

namespace byteforge::rendering
{
    struct BF_RENDERING_API opengl_render_pass_t
    {
    public:
        static void begin(const opengl_framebuffer_t& target, const math::vec4_t& clear_color);

        static void begin_window(const math::vec2i_t& window_size);

        static void begin_window(const math::vec2i_t& window_size, const math::vec4_t& clear_color);
    };
}

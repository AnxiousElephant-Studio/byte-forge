#pragma once

#include <byte-forge-engine/modules/rendering/rendering_api.h>
#include <byte-forge-engine/modules/math/rects/recti.h>
#include <byte-forge-engine/modules/math/vectors/vector4.h>

namespace byteforge::rendering
{
    struct BF_RENDERING_API opengl_render_state_t
    {
    public:
        static void enable_blending();
        static void disable_blending();
        static void use_alpha_blending();
        static void use_additive_blending();

        static void set_viewport(const math::recti_t& bounds);

        static void clear_color(const math::vec4_t& color);
        static void clear_depth_and_stencil();
    };
}

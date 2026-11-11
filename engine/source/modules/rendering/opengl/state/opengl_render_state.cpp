#include <byte-forge-engine/modules/rendering/opengl/state/opengl_render_state.h>

#include <glad/gl.h>

namespace byteforge::rendering
{
    void opengl_render_state_t::enable_blending()
    {
        glEnable(GL_BLEND);
    }

    void opengl_render_state_t::disable_blending()
    {
        glDisable(GL_BLEND);
    }

    void opengl_render_state_t::use_alpha_blending()
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void opengl_render_state_t::use_additive_blending()
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    }

    void opengl_render_state_t::set_viewport(const math::recti_t& bounds)
    {
        glViewport(bounds.position.x, bounds.position.y, bounds.size.width(), bounds.size.height());
    }

    void opengl_render_state_t::clear_color(const math::vec4_t& color)
    {
        glClearColor(color.x, color.y, color.z, color.w);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void opengl_render_state_t::clear_depth_and_stencil()
    {
        glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
}

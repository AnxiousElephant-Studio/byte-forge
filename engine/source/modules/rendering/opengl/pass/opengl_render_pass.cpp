#include <byte-forge-engine/modules/rendering/opengl/pass/opengl_render_pass.h>
#include <byte-forge-engine/modules/rendering/opengl/state/opengl_render_state.h>

namespace byteforge::rendering
{
    void opengl_render_pass_t::begin(const opengl_framebuffer_t& target, const math::vec4_t& clear_color)
    {
        target.bind();

        opengl_render_state_t::set_viewport(target.get_bounds());
        opengl_render_state_t::clear_color(clear_color);

        if (target.descriptor.use_depth_stencil)
        {
            opengl_render_state_t::clear_depth_and_stencil();
        }
    }

    void opengl_render_pass_t::begin_window(const math::vec2i_t& window_size)
    {
        opengl_framebuffer_t::bind_window();
        opengl_render_state_t::set_viewport(math::recti_t::from_size(window_size));
    }

    void opengl_render_pass_t::begin_window(const math::vec2i_t& window_size, const math::vec4_t& clear_color)
    {
        begin_window(window_size);
        opengl_render_state_t::clear_color(clear_color);
    }
}

#pragma once

#include <byte-forge-engine/modules/rendering/rendering_api.h>
#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/math/rects/recti.h>
#include <byte-forge-engine/modules/math/vectors/vector2i.h>
#include <byte-forge-engine/modules/rendering/opengl/framebuffer/opengl_framebuffer_descriptor.h>
#include <byte-forge-engine/modules/rendering/opengl/texture/opengl_texture.h>

namespace byteforge::rendering
{
    class BF_RENDERING_API opengl_framebuffer_t
    {
    public:
        opengl_framebuffer_descriptor_t descriptor;

        opengl_framebuffer_t()
            : descriptor(), framebuffer_id(0), depth_stencil_renderbuffer_id(0), color_texture()
        {
        }

        explicit opengl_framebuffer_t(const opengl_framebuffer_descriptor_t& descriptor)
            : descriptor(descriptor), framebuffer_id(0), depth_stencil_renderbuffer_id(0), color_texture()
        {
        }

        bool resize(const math::vec2i_t& size);
        void dispose();

        void bind() const;

        static void bind_window();

        const opengl_texture_t& get_color_texture() const { return color_texture; }
        math::vec2i_t get_size() const { return color_texture.size; }
        math::recti_t get_bounds() const { return math::recti_t::from_size(color_texture.size); }
        bool is_valid() const { return framebuffer_id != 0; }

    private:
        void create_depth_stencil_attachment(const math::vec2i_t& size);
        void dispose_depth_stencil_attachment();

        u32 framebuffer_id;
        u32 depth_stencil_renderbuffer_id;
        opengl_texture_t color_texture;
    };
}

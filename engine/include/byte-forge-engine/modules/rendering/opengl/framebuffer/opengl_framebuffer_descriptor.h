#pragma once

namespace byteforge::rendering
{
    struct opengl_framebuffer_descriptor_t
    {
    public:
        bool use_depth_stencil;
        bool read_color_as_opaque;

        constexpr opengl_framebuffer_descriptor_t() : use_depth_stencil(false), read_color_as_opaque(false) {}

        constexpr opengl_framebuffer_descriptor_t(bool use_depth_stencil, bool read_color_as_opaque)
            : use_depth_stencil(use_depth_stencil)
            , read_color_as_opaque(read_color_as_opaque)
        {
        }
    };
}

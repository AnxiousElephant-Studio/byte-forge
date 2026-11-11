#include <byte-forge-engine/modules/logging/log_macros.h>
#include <byte-forge-engine/modules/rendering/opengl/framebuffer/opengl_framebuffer.h>

#include <glad/gl.h>

namespace byteforge::rendering
{
    namespace
    {
        const char* const incomplete_error_format = "Could not create the framebuffer with size: %dx%d. Cause: incomplete attachment (status: 0x%X).";
    }

    bool opengl_framebuffer_t::resize(const math::vec2i_t& size)
    {
        if (is_valid() && color_texture.size == size)
        {
            return true;
        }

        if (framebuffer_id == 0)
        {
            glGenFramebuffers(1, &framebuffer_id);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);

        color_texture.create_empty(size);

        if (descriptor.read_color_as_opaque)
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ONE);
        }

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_texture.texture_id, 0);

        if (descriptor.use_depth_stencil)
        {
            create_depth_stencil_attachment(size);
        }

        const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

        bind_window();

        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            LOG_ERROR(incomplete_error_format, size.x, size.y, status);
            dispose();
            return false;
        }

        return true;
    }

    void opengl_framebuffer_t::create_depth_stencil_attachment(const math::vec2i_t& size)
    {
        dispose_depth_stencil_attachment();

        glGenRenderbuffers(1, &depth_stencil_renderbuffer_id);
        glBindRenderbuffer(GL_RENDERBUFFER, depth_stencil_renderbuffer_id);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size.x, size.y);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_stencil_renderbuffer_id);
    }

    void opengl_framebuffer_t::dispose_depth_stencil_attachment()
    {
        if (depth_stencil_renderbuffer_id != 0)
        {
            glDeleteRenderbuffers(1, &depth_stencil_renderbuffer_id);
            depth_stencil_renderbuffer_id = 0;
        }
    }

    void opengl_framebuffer_t::dispose()
    {
        dispose_depth_stencil_attachment();

        if (framebuffer_id != 0)
        {
            glDeleteFramebuffers(1, &framebuffer_id);
            framebuffer_id = 0;
        }

        if (color_texture.texture_id != 0)
        {
            color_texture.dispose();
        }
    }

    void opengl_framebuffer_t::bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);
    }

    void opengl_framebuffer_t::bind_window()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

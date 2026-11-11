#include <byte-forge-engine/modules/rendering/opengl/texture/opengl_texture.h>
#include <byte-forge-engine/modules/image/image.h>

#include <glad/gl.h>

namespace byteforge::rendering
{
    bool opengl_texture_t::load(const char* path)
    {
        image::image_t image;

        if (!image.load(path))
        {
            return false;
        }

        return upload(image.pixels, image.size);
    }

    bool opengl_texture_t::upload(const u8* rgba_pixels, const math::vec2i_t& pixel_size)
    {
        if (texture_id == 0)
        {
            glGenTextures(1, &texture_id);
        }

        size = pixel_size;

        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.width(), size.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_pixels);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        return true;
    }

    bool opengl_texture_t::create_empty(const math::vec2i_t& pixel_size)
    {
        return upload(nullptr, pixel_size);
    }

    void opengl_texture_t::bind(u32 slot) const
    {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, texture_id);
    }

    void opengl_texture_t::dispose()
    {
        glDeleteTextures(1, &texture_id);
        texture_id = 0;
    }
}

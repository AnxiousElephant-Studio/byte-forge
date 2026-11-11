#include <byte-forge-engine/modules/image/image.h>
#include <byte-forge-engine/modules/logging/log_macros.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace byteforge::image
{
    namespace
    {
        const i32 rgba_channels = 4;

        const char* const load_error_format = "Could not load image at path: '%s'. Cause: %s";
    }

    bool image_t::load(const char* path)
    {
        dispose();

        pixels = stbi_load(path, &size.x, &size.y, &channels, rgba_channels);

        if (pixels == nullptr)
        {
            LOG_ERROR(load_error_format, path, stbi_failure_reason());
            dispose();
            return false;
        }

        return true;
    }

    void image_t::dispose()
    {
        if (pixels != nullptr)
        {
            stbi_image_free(pixels);
            pixels = nullptr;
            channels = 0;
        }

        size = math::vec2i_t::zero();
    }
}

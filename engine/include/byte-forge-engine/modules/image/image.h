#pragma once

#include <byte-forge-engine/modules/image/image_api.h>
#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/math/vectors/vector2i.h>

namespace byteforge::image
{
    struct BF_IMAGE_API image_t
    {
    public:
        u8* pixels;
        math::vec2i_t size;
        i32 channels;

        image_t() : pixels(nullptr), size() {}

        ~image_t()
        {
            dispose();
        }

        image_t(const image_t&)            = delete;
        image_t& operator=(const image_t&) = delete;

        bool load(const char* path);
        void dispose();
    };
}

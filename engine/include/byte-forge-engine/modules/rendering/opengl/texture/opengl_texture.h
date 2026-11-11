#pragma once

#include <byte-forge-engine/modules/rendering/rendering_api.h>
#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/math/vectors/vector2i.h>

namespace byteforge::rendering
{
    struct BF_RENDERING_API opengl_texture_t
    {
    public:
        u32 texture_id;
        math::vec2i_t size;

        opengl_texture_t() : texture_id(0), size() {}

        bool load(const char* path);
        bool upload(const u8* rgba_pixels, const math::vec2i_t& pixel_size);
        bool create_empty(const math::vec2i_t& pixel_size);
        void dispose();

        void bind(u32 slot) const;
    };
}

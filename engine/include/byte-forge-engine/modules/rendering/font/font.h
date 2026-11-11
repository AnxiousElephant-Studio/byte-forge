#pragma once

#include <byte-forge-engine/modules/rendering/rendering_api.h>
#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/rendering/opengl/texture/opengl_texture.h>

namespace byteforge::rendering
{
    constexpr u32 FONT_FIRST_CHARACTER = 32;
    constexpr u32 FONT_CHARACTER_COUNT = 96;

    struct font_glyph_t
    {
        u16 x0, y0, x1, y1;
        f32 x_offset, y_offset, x_advance;
    };

    struct BF_RENDERING_API font_t
    {
    public:
        opengl_texture_t atlas;
        font_glyph_t glyphs[FONT_CHARACTER_COUNT];
        f32 size;

        font_t() : atlas(), glyphs(), size(0.0f) {}

        bool load(const char* path, const f32 font_size);
        void dispose();
    };
}

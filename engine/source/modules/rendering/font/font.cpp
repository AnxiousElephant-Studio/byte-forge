#include <byte-forge-engine/modules/rendering/font/font.h>
#include <byte-forge-engine/modules/io/file.h>
#include <byte-forge-engine/modules/io/e_file_data_types.h>
#include <byte-forge-engine/modules/logging/log_macros.h>
#include <byte-forge-engine/modules/math/vectors/vector2i.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include <vector>

namespace byteforge::rendering
{
    namespace
    {
        const i32 atlas_size      = 512;
        const u8  white_channel   = 255;
        const usize rgba_channels = 4;

        const char* const read_error_format = "Could not read font at path: '%s'.";
        const char* const bake_error_format = "Could not bake font at path: '%s' with size: %.1f. Cause: the glyphs do not fit into the atlas.";
    }

    bool font_t::load(const char* path, const f32 font_size)
    {
        dispose();

        io::file_t file;

        if (!file.read(path, io::e_file_data_types::Binary))
        {
            LOG_ERROR(read_error_format, path);
            return false;
        }

        std::vector<u8> coverage(static_cast<usize>(atlas_size * atlas_size));
        stbtt_bakedchar baked_characters[FONT_CHARACTER_COUNT];

        const i32 result = stbtt_BakeFontBitmap(
            reinterpret_cast<const u8*>(file.content), 0, font_size,
            coverage.data(), atlas_size, atlas_size,
            static_cast<i32>(FONT_FIRST_CHARACTER), static_cast<i32>(FONT_CHARACTER_COUNT),
            baked_characters
        );

        if (result <= 0)
        {
            LOG_ERROR(bake_error_format, path, font_size);
            return false;
        }

        std::vector<u8> rgba_pixels(coverage.size() * rgba_channels);

        for (usize index = 0; index < coverage.size(); index++)
        {
            rgba_pixels[index * rgba_channels + 0] = white_channel;
            rgba_pixels[index * rgba_channels + 1] = white_channel;
            rgba_pixels[index * rgba_channels + 2] = white_channel;
            rgba_pixels[index * rgba_channels + 3] = coverage[index];
        }

        for (u32 index = 0; index < FONT_CHARACTER_COUNT; index++)
        {
            glyphs[index].x0        = baked_characters[index].x0;
            glyphs[index].y0        = baked_characters[index].y0;
            glyphs[index].x1        = baked_characters[index].x1;
            glyphs[index].y1        = baked_characters[index].y1;
            glyphs[index].x_offset  = baked_characters[index].xoff;
            glyphs[index].y_offset  = baked_characters[index].yoff;
            glyphs[index].x_advance = baked_characters[index].xadvance;
        }

        size = font_size;

        return atlas.upload(rgba_pixels.data(), math::vec2i_t(atlas_size, atlas_size));
    }

    void font_t::dispose()
    {
        atlas.dispose();
        size = 0.0f;
    }
}

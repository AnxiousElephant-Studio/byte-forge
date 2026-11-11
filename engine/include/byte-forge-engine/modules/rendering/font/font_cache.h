#pragma once

#include <byte-forge-engine/modules/rendering/rendering_api.h>
#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/rendering/font/font.h>

#include <string>
#include <unordered_map>

namespace byteforge::rendering
{
    class BF_RENDERING_API font_cache_t
    {
    public:
        void dispose();

        const font_t* get(const char* path, const f32 size);

        usize get_count() const;
        usize get_failed_count() const;

    private:
        std::unordered_map<std::string, font_t*> fonts;
    };
}

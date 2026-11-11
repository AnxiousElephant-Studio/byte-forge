#pragma once

#include <byte-forge-engine/modules/rendering/rendering_api.h>
#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/rendering/opengl/texture/opengl_texture.h>

#include <string>
#include <unordered_map>

namespace byteforge::rendering
{
    class BF_RENDERING_API texture_cache_t
    {
    public:
        void dispose();

        const opengl_texture_t* get(const char* path);
        void invalidate(const char* path);

        usize get_count() const;
        usize get_failed_count() const;

    private:
        std::unordered_map<std::string, opengl_texture_t> textures;
    };
}

#include <byte-forge-engine/modules/rendering/texture/texture_cache.h>

namespace byteforge::rendering
{
    const opengl_texture_t* texture_cache_t::get(const char* path)
    {
        if (path == nullptr || path[0] == '\0')
        {
            return nullptr;
        }

        std::unordered_map<std::string, opengl_texture_t>::iterator iterator = textures.find(path);

        if (iterator == textures.end())
        {
            opengl_texture_t texture;
            texture.load(path);

            iterator = textures.emplace(path, texture).first;
        }

        return iterator->second.texture_id != 0 ? &iterator->second : nullptr;
    }

    usize texture_cache_t::get_count() const
    {
        return textures.size();
    }

    usize texture_cache_t::get_failed_count() const
    {
        usize failed_count = 0;

        for (const std::pair<const std::string, opengl_texture_t>& entry : textures)
        {
            if (entry.second.texture_id == 0)
            {
                failed_count++;
            }
        }

        return failed_count;
    }

    void texture_cache_t::invalidate(const char* path)
    {
        std::unordered_map<std::string, opengl_texture_t>::iterator iterator = textures.find(path);

        if (iterator != textures.end())
        {
            iterator->second.dispose();
            textures.erase(iterator);
        }
    }

    void texture_cache_t::dispose()
    {
        for (std::pair<const std::string, opengl_texture_t>& entry : textures)
        {
            entry.second.dispose();
        }

        textures.clear();
    }
}

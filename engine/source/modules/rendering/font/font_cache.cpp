#include <byte-forge-engine/modules/rendering/font/font_cache.h>

#include <cstdio>

namespace byteforge::rendering
{
    namespace
    {
        const usize key_buffer_size = 300;

        const char* const key_format = "%s@%.1f";
    }

    const font_t* font_cache_t::get(const char* path, const f32 size)
    {
        if (path == nullptr || path[0] == '\0' || size <= 0.0f)
        {
            return nullptr;
        }

        char key[key_buffer_size];
        std::snprintf(key, sizeof(key), key_format, path, size);

        std::unordered_map<std::string, font_t*>::iterator iterator = fonts.find(key);

        if (iterator == fonts.end())
        {
            font_t* font = new font_t();
            font->load(path, size);

            iterator = fonts.emplace(key, font).first;
        }

        return iterator->second->atlas.texture_id != 0 ? iterator->second : nullptr;
    }

    usize font_cache_t::get_count() const
    {
        return fonts.size();
    }

    usize font_cache_t::get_failed_count() const
    {
        usize failed_count = 0;

        for (const std::pair<const std::string, font_t*>& entry : fonts)
        {
            if (entry.second->atlas.texture_id == 0)
            {
                failed_count++;
            }
        }

        return failed_count;
    }

    void font_cache_t::dispose()
    {
        for (std::pair<const std::string, font_t*>& entry : fonts)
        {
            entry.second->dispose();
            delete entry.second;
        }

        fonts.clear();
    }
}

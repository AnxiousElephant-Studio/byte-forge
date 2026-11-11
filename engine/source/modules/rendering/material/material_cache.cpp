#include <byte-forge-engine/modules/rendering/material/material_cache.h>
#include <byte-forge-engine/modules/io/file_system.h>

namespace byteforge::rendering
{
    namespace
    {
        const char* const key_separator = "#";

        std::string make_key(const char* path, const e_material_shading_models shading_model)
        {
            return std::string(path) + key_separator + get_material_shading_model_name(shading_model);
        }
    }

    material_t* material_cache_t::get(const char* path, const e_material_shading_models shading_model)
    {
        if (path == nullptr || path[0] == '\0')
        {
            return nullptr;
        }

        const std::string key = make_key(path, shading_model);
        std::unordered_map<std::string, cached_material_t>::iterator iterator = materials.find(key);

        if (iterator == materials.end())
        {
            cached_material_t entry = { material_t(), path, shading_model, io::file_system_t::get_last_write_time(path) };
            iterator = materials.emplace(key, entry).first;
            iterator->second.material.load(path, shading_model);
        }

        return iterator->second.material.is_loaded() ? &iterator->second.material : nullptr;
    }

    bool material_cache_t::is_failed(const char* path, const e_material_shading_models shading_model) const
    {
        const std::unordered_map<std::string, cached_material_t>::const_iterator iterator = materials.find(make_key(path, shading_model));

        return iterator != materials.end() && !iterator->second.material.is_loaded();
    }

    void material_cache_t::invalidate(const char* path)
    {
        for (std::unordered_map<std::string, cached_material_t>::iterator iterator = materials.begin(); iterator != materials.end();)
        {
            if (iterator->second.path == path)
            {
                iterator->second.material.dispose();
                iterator = materials.erase(iterator);
            }
            else
            {
                ++iterator;
            }
        }
    }

    usize material_cache_t::reload_modified()
    {
        usize reloaded_count = 0;

        for (std::pair<const std::string, cached_material_t>& entry : materials)
        {
            cached_material_t& cached = entry.second;
            const i64 write_time = io::file_system_t::get_last_write_time(cached.path.c_str());

            if (write_time == cached.last_write_time)
            {
                continue;
            }

            cached.last_write_time = write_time;

            if (cached.material.is_loaded() ? cached.material.reload() : cached.material.load(cached.path.c_str(), cached.shading_model))
            {
                reloaded_count++;
            }
        }

        return reloaded_count;
    }

    usize material_cache_t::get_count() const
    {
        return materials.size();
    }

    usize material_cache_t::get_failed_count() const
    {
        usize failed_count = 0;

        for (const std::pair<const std::string, cached_material_t>& entry : materials)
        {
            if (!entry.second.material.is_loaded())
            {
                failed_count++;
            }
        }

        return failed_count;
    }

    void material_cache_t::dispose()
    {
        for (std::pair<const std::string, cached_material_t>& entry : materials)
        {
            entry.second.material.dispose();
        }

        materials.clear();
    }
}

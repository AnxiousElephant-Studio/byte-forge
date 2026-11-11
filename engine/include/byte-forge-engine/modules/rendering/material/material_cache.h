#pragma once

#include <byte-forge-engine/modules/rendering/rendering_api.h>
#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/rendering/material/e_material_shading_models.h>
#include <byte-forge-engine/modules/rendering/material/material.h>

#include <string>
#include <unordered_map>

namespace byteforge::rendering
{
    class BF_RENDERING_API material_cache_t
    {
    public:
        void dispose();

        material_t* get(const char* path, const e_material_shading_models shading_model);
        bool is_failed(const char* path, const e_material_shading_models shading_model) const;
        void invalidate(const char* path);
        usize reload_modified();

        usize get_count() const;
        usize get_failed_count() const;

    private:
        struct cached_material_t
        {
            material_t material;
            std::string path;
            e_material_shading_models shading_model;
            i64 last_write_time;
        };

        std::unordered_map<std::string, cached_material_t> materials;
    };
}

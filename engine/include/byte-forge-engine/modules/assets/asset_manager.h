#pragma once

#include <byte-forge-engine/modules/assets/assets_api.h>
#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/rendering/font/font_cache.h>
#include <byte-forge-engine/modules/rendering/material/material_cache.h>
#include <byte-forge-engine/modules/rendering/texture/texture_cache.h>

namespace byteforge::assets
{
    class BF_ASSETS_API asset_manager_t
    {
    public:
        rendering::texture_cache_t texture_cache;
        rendering::font_cache_t font_cache;
        rendering::material_cache_t material_cache;

        asset_manager_t() : texture_cache(), font_cache(), material_cache() {}

        asset_manager_t(const asset_manager_t&)            = delete;
        asset_manager_t& operator=(const asset_manager_t&) = delete;

        void dispose();

        usize get_loaded_count() const;
        usize get_failed_count() const;
    };
}

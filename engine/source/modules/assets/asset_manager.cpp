#include <byte-forge-engine/modules/assets/asset_manager.h>

namespace byteforge::assets
{
    void asset_manager_t::dispose()
    {
        material_cache.dispose();
        font_cache.dispose();
        texture_cache.dispose();
    }

    usize asset_manager_t::get_loaded_count() const
    {
        return texture_cache.get_count() + font_cache.get_count() + material_cache.get_count();
    }

    usize asset_manager_t::get_failed_count() const
    {
        return texture_cache.get_failed_count() + font_cache.get_failed_count() + material_cache.get_failed_count();
    }
}

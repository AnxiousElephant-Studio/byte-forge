#include <byte-forge-engine/modules/ecs/system.h>
#include <byte-forge-engine/modules/logging/log_macros.h>

namespace byteforge::ecs
{
    namespace
    {
        const char* const header_format   = "==================== ECS System Debug ====================";
        const char* const index_format    = "Index           : %zu";
        const char* const id_format       = "System ID       : %llu";
        const char* const name_format     = "System Name     : %s";
        const char* const stage_format    = "Stage / Order   : %s / %d";
        const char* const bitset_format   = "Required Bitset : 0x%016llX";
        const char* const entities_format = "Entities        : %zu";
        const char* const footer_format   = "==========================================================";
    }

    bool system_t::accepts(const u64 entity_components_bitset) const
    {
        return (required_entity_components_bitset & entity_components_bitset) == required_entity_components_bitset;
    }

    bool system_t::contains_entity(const entity_t entity) const
    {
        for (usize index = 0; index < entities.size(); index++)
        {
            if (entities[index] == entity)
            {
                return true;
            }
        }

        return false;
    }

    void system_t::add_entity(const entity_t entity)
    {
        entities.push_back(entity);
    }

    void system_t::remove_entity(const entity_t entity)
    {
        for (usize index = 0; index < entities.size(); index++)
        {
            if (entities[index] == entity)
            {
                entities.erase(entities.begin() + static_cast<isize>(index));
                return;
            }
        }
    }

    void system_t::print_info(const usize registry_index) const
    {
        LOG_DEBUG(header_format);
        LOG_DEBUG(index_format, registry_index);
        LOG_DEBUG(id_format, static_cast<unsigned long long>(id));
        LOG_DEBUG(name_format, name.c_str());
        LOG_DEBUG(stage_format, get_system_stage_name(stage), order);
        LOG_DEBUG(bitset_format, static_cast<unsigned long long>(required_entity_components_bitset));
        LOG_DEBUG(entities_format, entities.size());
        LOG_DEBUG(footer_format);
    }

    void system_t::dispose()
    {
        if (free_function != nullptr && data != nullptr)
        {
            free_function(data);
        }

        data = nullptr;
        entities.clear();
    }
}

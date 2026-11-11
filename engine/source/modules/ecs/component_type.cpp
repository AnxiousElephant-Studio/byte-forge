#include <byte-forge-engine/modules/ecs/component_type.h>
#include <byte-forge-engine/modules/logging/log_macros.h>

#include <cstring>

namespace byteforge::ecs
{
    namespace
    {
        const char* const header_format    = "================== ECS Component Type Debug ==================";
        const char* const index_format     = "Index                        : %zu";
        const char* const id_format        = "Component Type ID            : %llu";
        const char* const size_format      = "Component Type Size          : %zu bytes";
        const char* const name_format      = "Component Type Name          : %s";
        const char* const count_format     = "Component Array Size         : %zu";
        const char* const map_entry_format = "Entity ID: %llu -> Array Index: %zu";
        const char* const footer_format    = "==============================================================";
    }

    usize component_type_t::get_count() const
    {
        return size == 0 ? 0 : data.size() / size;
    }

    usize component_type_t::get_memory_bytes() const
    {
        return data.capacity() + entity_id_to_index.size() * (sizeof(u64) + sizeof(usize));
    }

    void* component_type_t::get(const entity_t entity)
    {
        std::unordered_map<u64, usize>::iterator iterator = entity_id_to_index.find(entity.id);

        if (iterator == entity_id_to_index.end())
        {
            return nullptr;
        }

        return data.data() + iterator->second * size;
    }

    bool component_type_t::has(const entity_t entity) const
    {
        return entity_id_to_index.find(entity.id) != entity_id_to_index.end();
    }

    void component_type_t::add_or_replace(const entity_t entity, const void* const component)
    {
        void* existing = get(entity);

        if (existing != nullptr)
        {
            if (free_function != nullptr)
            {
                free_function(existing);
            }

            std::memcpy(existing, component, size);
            return;
        }

        const usize index = get_count();

        data.resize(data.size() + size);
        std::memcpy(data.data() + index * size, component, size);

        entity_id_to_index[entity.id] = index;
    }

    void component_type_t::remove(const entity_t entity)
    {
        std::unordered_map<u64, usize>::iterator iterator = entity_id_to_index.find(entity.id);

        if (iterator == entity_id_to_index.end())
        {
            return;
        }

        const usize removed_index = iterator->second;
        u8* removed_data = data.data() + removed_index * size;

        if (free_function != nullptr)
        {
            free_function(removed_data);
        }

        data.erase(data.begin() + static_cast<isize>(removed_index * size), data.begin() + static_cast<isize>((removed_index + 1) * size));
        entity_id_to_index.erase(iterator);

        for (std::pair<const u64, usize>& entry : entity_id_to_index)
        {
            if (entry.second > removed_index)
            {
                entry.second--;
            }
        }
    }

    void component_type_t::print_info(const usize registry_index) const
    {
        LOG_DEBUG(header_format);
        LOG_DEBUG(index_format, registry_index);
        LOG_DEBUG(id_format, static_cast<unsigned long long>(id));
        LOG_DEBUG(size_format, size);
        LOG_DEBUG(name_format, name.c_str());
        LOG_DEBUG(count_format, get_count());

        for (const std::pair<const u64, usize>& entry : entity_id_to_index)
        {
            LOG_DEBUG(map_entry_format, static_cast<unsigned long long>(entry.first), entry.second);
        }

        LOG_DEBUG(footer_format);
    }

    void component_type_t::dispose()
    {
        if (free_function != nullptr)
        {
            for (usize index = 0; index < get_count(); index++)
            {
                free_function(data.data() + index * size);
            }
        }

        data.clear();
        entity_id_to_index.clear();
    }
}

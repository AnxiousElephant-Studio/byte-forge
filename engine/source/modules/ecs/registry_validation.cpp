#include <byte-forge-engine/modules/ecs/registry.h>

#include <cstdio>
#include <unordered_set>

namespace byteforge::ecs
{
    namespace
    {
        const usize issue_buffer_size = 256;

        const char* const bitsets_count_format    = "Invalid entity bitset count. Cause: %zu bitsets exist, but %llu entity IDs were created.";
        const char* const invalid_size_format     = "Invalid component type: '%s'. Cause: invalid size of %zu bytes.";
        const char* const data_size_format        = "Invalid component type: '%s'. Cause: buffer of %zu bytes is not a multiple of the element size of %zu bytes.";
        const char* const map_size_format         = "Invalid component type: '%s'. Cause: %zu entries in the entity map, but %zu components stored.";
        const char* const index_range_format      = "Invalid component type: '%s'. Cause: entity ID: %llu points to slot %zu, out of %zu slots.";
        const char* const duplicate_slot_format   = "Invalid component type: '%s'. Cause: slot %zu is used by more than one entity.";
        const char* const dead_entity_format      = "Invalid component type: '%s'. Cause: entity ID: %llu has the component, but is not alive.";
        const char* const missing_bit_format      = "Invalid component type: '%s'. Cause: entity ID: %llu has the component, but its bit is not set.";
        const char* const missing_data_format     = "Invalid entity ID: %llu. Cause: the bit of component type: '%s' is set, but no component is stored.";
        const char* const unknown_bit_format      = "Invalid entity ID: %llu. Cause: bit %zu is set, but no component type has that ID.";
        const char* const dead_in_system_format   = "Invalid system: '%s'. Cause: holds entity ID: %llu, which is not alive.";
        const char* const mismatch_format         = "Invalid system: '%s'. Cause: holds entity ID: %llu, which does not have the required components.";
        const char* const duplicate_entity_format = "Invalid system: '%s'. Cause: holds entity ID: %llu more than once.";
        const char* const not_in_system_format    = "Invalid system: '%s'. Cause: entity ID: %llu matches the system, but is not in it.";
    }

    usize registry_t::validate(std::vector<std::string>& out_issues) const
    {
        out_issues.clear();

        char issue[issue_buffer_size];

        std::unordered_set<u64> alive_entities;
        std::unordered_set<u64> pending_entities;

        for (usize index = 0; index < entities.size(); index++)
        {
            alive_entities.insert(entities[index].id);
        }

        for (usize index = 0; index < entities_to_add.size(); index++)
        {
            pending_entities.insert(entities_to_add[index].id);
        }

        if (entities_components_bitsets.size() != entities_current_id)
        {
            std::snprintf(issue, sizeof(issue), bitsets_count_format, entities_components_bitsets.size(), static_cast<unsigned long long>(entities_current_id));
            out_issues.push_back(issue);
        }

        for (usize type_index = 0; type_index < component_types.size(); type_index++)
        {
            const component_type_t& type = *component_types[type_index];

            if (type.size == 0)
            {
                std::snprintf(issue, sizeof(issue), invalid_size_format, type.name.c_str(), type.size);
                out_issues.push_back(issue);
                continue;
            }

            const usize count = type.get_count();

            if (type.data.size() % type.size != 0)
            {
                std::snprintf(issue, sizeof(issue), data_size_format, type.name.c_str(), type.data.size(), type.size);
                out_issues.push_back(issue);
            }

            if (type.entity_id_to_index.size() != count)
            {
                std::snprintf(issue, sizeof(issue), map_size_format, type.name.c_str(), type.entity_id_to_index.size(), count);
                out_issues.push_back(issue);
            }

            std::vector<bool> used_slots(count, false);

            for (const std::pair<const u64, usize>& entry : type.entity_id_to_index)
            {
                if (entry.second >= count)
                {
                    std::snprintf(issue, sizeof(issue), index_range_format, type.name.c_str(), static_cast<unsigned long long>(entry.first), entry.second, count);
                    out_issues.push_back(issue);
                }
                else if (used_slots[entry.second])
                {
                    std::snprintf(issue, sizeof(issue), duplicate_slot_format, type.name.c_str(), entry.second);
                    out_issues.push_back(issue);
                }
                else
                {
                    used_slots[entry.second] = true;
                }

                if (alive_entities.find(entry.first) == alive_entities.end())
                {
                    std::snprintf(issue, sizeof(issue), dead_entity_format, type.name.c_str(), static_cast<unsigned long long>(entry.first));
                    out_issues.push_back(issue);
                }
                else if (entry.first < entities_components_bitsets.size() && (entities_components_bitsets[entry.first] & (1ULL << type.id)) == 0)
                {
                    std::snprintf(issue, sizeof(issue), missing_bit_format, type.name.c_str(), static_cast<unsigned long long>(entry.first));
                    out_issues.push_back(issue);
                }
            }
        }

        for (usize index = 0; index < entities.size(); index++)
        {
            const entity_t entity = entities[index];

            if (entity.id >= entities_components_bitsets.size())
            {
                continue;
            }

            const u64 bitset = entities_components_bitsets[entity.id];

            for (usize bit = 0; bit < MAX_COMPONENT_TYPES; bit++)
            {
                if ((bitset & (1ULL << bit)) == 0)
                {
                    continue;
                }

                if (bit >= component_types.size())
                {
                    std::snprintf(issue, sizeof(issue), unknown_bit_format, static_cast<unsigned long long>(entity.id), bit);
                    out_issues.push_back(issue);
                }
                else if (!component_types[bit]->has(entity))
                {
                    std::snprintf(issue, sizeof(issue), missing_data_format, static_cast<unsigned long long>(entity.id), component_types[bit]->name.c_str());
                    out_issues.push_back(issue);
                }
            }
        }

        for (usize system_index = 0; system_index < systems.size(); system_index++)
        {
            const system_t& system = *systems[system_index];

            std::unordered_set<u64> system_entities;

            for (usize index = 0; index < system.entities.size(); index++)
            {
                const entity_t entity = system.entities[index];

                if (!system_entities.insert(entity.id).second)
                {
                    std::snprintf(issue, sizeof(issue), duplicate_entity_format, system.name.c_str(), static_cast<unsigned long long>(entity.id));
                    out_issues.push_back(issue);
                }

                if (alive_entities.find(entity.id) == alive_entities.end())
                {
                    std::snprintf(issue, sizeof(issue), dead_in_system_format, system.name.c_str(), static_cast<unsigned long long>(entity.id));
                    out_issues.push_back(issue);
                }
                else if (entity.id < entities_components_bitsets.size() && !system.accepts(entities_components_bitsets[entity.id]))
                {
                    std::snprintf(issue, sizeof(issue), mismatch_format, system.name.c_str(), static_cast<unsigned long long>(entity.id));
                    out_issues.push_back(issue);
                }
            }

            for (usize index = 0; index < entities.size(); index++)
            {
                const entity_t entity = entities[index];

                if (entity.id < entities_components_bitsets.size()
                    && pending_entities.find(entity.id) == pending_entities.end()
                    && system.accepts(entities_components_bitsets[entity.id])
                    && system_entities.find(entity.id) == system_entities.end())
                {
                    std::snprintf(issue, sizeof(issue), not_in_system_format, static_cast<unsigned long long>(entity.id), system.name.c_str());
                    out_issues.push_back(issue);
                }
            }
        }

        return out_issues.size();
    }

    usize registry_t::get_memory_bytes() const
    {
        usize bytes = entities.capacity() * sizeof(entity_t)
            + entities_components_bitsets.capacity() * sizeof(u64)
            + entities_to_add.capacity() * sizeof(entity_t)
            + entities_to_kill.capacity() * sizeof(entity_t);

        for (usize index = 0; index < component_types.size(); index++)
        {
            bytes += component_types[index]->get_memory_bytes();
        }

        for (usize index = 0; index < systems.size(); index++)
        {
            bytes += systems[index]->entities.capacity() * sizeof(entity_t);
        }

        return bytes;
    }
}

#include <byte-forge-engine/modules/ecs/registry.h>

#include <algorithm>
#include <cstring>

namespace byteforge::ecs
{
    namespace
    {
        bool runs_before(const system_t* first, const system_t* second)
        {
            return first->stage < second->stage || (first->stage == second->stage && first->order < second->order);
        }
    }

    system_t* registry_t::create_system(const system_descriptor_t& descriptor, const u64 required_entity_components_bitset)
    {
        system_t* system = new system_t(systems.size(), descriptor.name.c_str(), descriptor.stage, descriptor.order, descriptor.data, required_entity_components_bitset, descriptor.update_function, descriptor.free_function);

        systems.insert(std::upper_bound(systems.begin(), systems.end(), system, runs_before), system);

        for (usize index = 0; index < entities.size(); index++)
        {
            const entity_t entity = entities[index];

            if (entity.id < entities_components_bitsets.size() && system->accepts(entities_components_bitsets[entity.id]))
            {
                system->add_entity(entity);
            }
        }

        return system;
    }

    bool registry_t::remove_system(const char* name)
    {
        for (usize index = 0; index < systems.size(); index++)
        {
            if (std::strcmp(systems[index]->name.c_str(), name) != 0)
            {
                continue;
            }

            systems[index]->dispose();
            delete systems[index];
            systems.erase(systems.begin() + static_cast<isize>(index));

            return true;
        }

        return false;
    }

    void registry_t::sort_systems()
    {
        std::stable_sort(systems.begin(), systems.end(), runs_before);
    }

    system_t* registry_t::find_system(const char* name) const
    {
        for (usize index = 0; index < systems.size(); index++)
        {
            if (std::strcmp(systems[index]->name.c_str(), name) == 0)
            {
                return systems[index];
            }
        }

        return nullptr;
    }
}

#include <byte-forge-engine/modules/ecs/registry.h>

namespace byteforge::ecs
{
    entity_t registry_t::create_entity()
    {
        entity_t entity(entities_current_id++);

        entities.push_back(entity);
        entities_components_bitsets.push_back(0);

        return entity;
    }

    void registry_t::publish_entity(const entity_t entity)
    {
        entities_to_add.push_back(entity);
    }

    void registry_t::spawn_entities()
    {
        for (usize index = 0; index < entities_to_add.size(); index++)
        {
            spawn_entity(entities_to_add[index]);
        }

        entities_to_add.clear();
    }

    void registry_t::spawn_entity(const entity_t entity)
    {
        if (entity.id >= entities_components_bitsets.size())
        {
            return;
        }

        const u64 entity_components_bitset = entities_components_bitsets[entity.id];

        for (usize index = 0; index < systems.size(); index++)
        {
            if (systems[index]->accepts(entity_components_bitset))
            {
                systems[index]->add_entity(entity);
            }
        }
    }

    void registry_t::refresh_entity_systems(const entity_t entity)
    {
        if (entity.id >= entities_components_bitsets.size())
        {
            return;
        }

        const u64 entity_components_bitset = entities_components_bitsets[entity.id];

        for (usize index = 0; index < systems.size(); index++)
        {
            system_t* system = systems[index];
            const bool accepts  = system->accepts(entity_components_bitset);
            const bool contains = system->contains_entity(entity);

            if (accepts && !contains)
            {
                system->add_entity(entity);
            }
            else if (!accepts && contains)
            {
                system->remove_entity(entity);
            }
        }
    }

    void registry_t::add_entity_to_kill_queue(const entity_t entity)
    {
        entities_to_kill.push_back(entity);
    }

    void registry_t::kill_entities()
    {
        for (usize index = 0; index < entities_to_kill.size(); index++)
        {
            kill_entity(entities_to_kill[index]);
        }

        entities_to_kill.clear();
    }

    void registry_t::kill_entity(const entity_t entity)
    {
        for (usize index = 0; index < systems.size(); index++)
        {
            systems[index]->remove_entity(entity);
        }

        for (usize index = 0; index < component_types.size(); index++)
        {
            component_types[index]->remove(entity);
        }

        if (entity.id < entities_components_bitsets.size())
        {
            entities_components_bitsets[entity.id] = 0;
        }

        for (usize index = 0; index < entities.size(); index++)
        {
            if (entities[index] == entity)
            {
                entities.erase(entities.begin() + static_cast<isize>(index));
                break;
            }
        }
    }
}

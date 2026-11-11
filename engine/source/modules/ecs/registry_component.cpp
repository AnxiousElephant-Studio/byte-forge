#include <byte-forge-engine/modules/ecs/registry.h>
#include <byte-forge-engine/modules/logging/log_macros.h>

namespace byteforge::ecs
{
    namespace
    {
        const char* const component_limit_error_format = "Could not register component type: '%s'. Cause: the limit of %zu component types was reached.";
        const char* const unknown_entity_error_format  = "Could not add component: '%s' to entity ID: %llu. Cause: the entity does not exist.";
    }

    component_type_t* registry_t::register_component_type(const usize size, const char* name, component_free_function_t free_function)
    {
        if (component_types.size() >= MAX_COMPONENT_TYPES)
        {
            LOG_ERROR(component_limit_error_format, name, MAX_COMPONENT_TYPES);
            return nullptr;
        }

        component_type_t* component_type = new component_type_t(component_types.size(), size, name, free_function);
        component_types.push_back(component_type);

        return component_type;
    }

    component_type_t* registry_t::get_component_type_by_name(const char* name) const
    {
        for (usize index = 0; index < component_types.size(); index++)
        {
            if (component_types[index]->name == name)
            {
                return component_types[index];
            }
        }

        return nullptr;
    }

    void registry_t::add_component_to_entity(component_type_t& component_type, const void* const component, const entity_t entity)
    {
        if (entity.id >= entities_components_bitsets.size())
        {
            LOG_ERROR(unknown_entity_error_format, component_type.name.c_str(), static_cast<unsigned long long>(entity.id));
            return;
        }

        entities_components_bitsets[entity.id] |= 1ULL << component_type.id;
        component_type.add_or_replace(entity, component);
    }

    void registry_t::remove_component_from_entity(component_type_t& component_type, const entity_t entity)
    {
        if (entity.id >= entities_components_bitsets.size())
        {
            return;
        }

        entities_components_bitsets[entity.id] &= ~(1ULL << component_type.id);
        component_type.remove(entity);
    }
}

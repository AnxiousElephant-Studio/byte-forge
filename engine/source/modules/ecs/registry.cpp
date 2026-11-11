#include <byte-forge-engine/modules/ecs/registry.h>
#include <byte-forge-engine/modules/logging/log_macros.h>

namespace byteforge::ecs
{
    namespace
    {
        const char* const entities_title_format        = "ECS Registry entities: %zu";
        const char* const systems_title_format         = "ECS Registry systems:";
        const char* const component_types_title_format = "ECS Registry component types:";
    }

    void registry_t::run_systems(const e_system_stages stage, const f64 delta_time)
    {
        for (usize index = 0; index < systems.size(); index++)
        {
            system_t* system = systems[index];

            if (system->stage == stage && system->is_enabled && system->update_function != nullptr)
            {
                system->update_function(*this, *system, delta_time);
            }
        }
    }

    void registry_t::print_info() const
    {
        LOG_INFO(entities_title_format, entities.size());

        LOG_INFO(systems_title_format);
        for (usize index = 0; index < systems.size(); index++)
        {
            systems[index]->print_info(index);
        }

        LOG_INFO(component_types_title_format);
        for (usize index = 0; index < component_types.size(); index++)
        {
            component_types[index]->print_info(index);
        }
    }

    void registry_t::dispose()
    {
        for (usize index = 0; index < systems.size(); index++)
        {
            systems[index]->dispose();
            delete systems[index];
        }

        for (usize index = 0; index < component_types.size(); index++)
        {
            component_types[index]->dispose();
            delete component_types[index];
        }

        systems.clear();
        component_types.clear();
        entities.clear();
        entities_components_bitsets.clear();
        entities_to_add.clear();
        entities_to_kill.clear();
        entities_current_id = 0;
    }
}

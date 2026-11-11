#include <byte-forge-engine/modules/ecs/system_schedule.h>
#include <byte-forge-engine/modules/logging/log_macros.h>

namespace byteforge::ecs
{
    namespace
    {
        const char* const missing_component_type_error_format = "Could not attach system: '%s'. Cause: component type: '%s' is not registered.";
        const char* const duplicate_system_error_format       = "Could not add system: '%s'. Cause: a system with this name is already in the schedule.";
    }

    void system_schedule_t::clear()
    {
        system_descriptors.clear();
    }

    bool system_schedule_t::add_system(const system_descriptor_t& descriptor)
    {
        if (find_system_descriptor(descriptor.name.c_str()) != nullptr)
        {
            LOG_ERROR(duplicate_system_error_format, descriptor.name.c_str());
            return false;
        }

        system_descriptors.push_back(descriptor);

        return true;
    }

    bool system_schedule_t::remove_system(const char* name)
    {
        for (usize index = 0; index < system_descriptors.size(); index++)
        {
            if (system_descriptors[index].name == name)
            {
                system_descriptors.erase(system_descriptors.begin() + static_cast<isize>(index));
                return true;
            }
        }

        return false;
    }

    bool system_schedule_t::attach(registry_t& registry) const
    {
        bool attached_all = true;

        for (usize index = 0; index < system_descriptors.size(); index++)
        {
            attached_all = attach_system(registry, system_descriptors[index]) && attached_all;
        }

        return attached_all;
    }

    bool system_schedule_t::attach_system(registry_t& registry, const system_descriptor_t& descriptor) const
    {
        u64 required_entity_components_bitset = 0;

        for (usize index = 0; index < descriptor.required_component_type_names.size(); index++)
        {
            const std::string& component_type_name = descriptor.required_component_type_names[index];
            const component_type_t* component_type = registry.get_component_type_by_name(component_type_name.c_str());

            if (component_type == nullptr)
            {
                LOG_ERROR(missing_component_type_error_format, descriptor.name.c_str(), component_type_name.c_str());
                return false;
            }

            required_entity_components_bitset |= 1ULL << component_type->id;
        }

        registry.create_system(descriptor, required_entity_components_bitset);

        return true;
    }

    system_descriptor_t* system_schedule_t::find_system_descriptor(const char* name)
    {
        for (usize index = 0; index < system_descriptors.size(); index++)
        {
            if (system_descriptors[index].name == name)
            {
                return &system_descriptors[index];
            }
        }

        return nullptr;
    }

    const std::vector<system_descriptor_t>& system_schedule_t::get_system_descriptors() const
    {
        return system_descriptors;
    }

    bool system_schedule_t::is_empty() const
    {
        return system_descriptors.empty();
    }
}

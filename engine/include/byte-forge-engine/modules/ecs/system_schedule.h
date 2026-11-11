#pragma once

#include <byte-forge-engine/modules/ecs/ecs_api.h>
#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/ecs/registry.h>
#include <byte-forge-engine/modules/ecs/system_descriptor.h>

#include <vector>

namespace byteforge::ecs
{
    class BF_ECS_API system_schedule_t
    {
    public:
        system_schedule_t() : system_descriptors() {}

        void clear();

        bool add_system(const system_descriptor_t& descriptor);
        bool remove_system(const char* name);

        bool attach(registry_t& registry) const;
        bool attach_system(registry_t& registry, const system_descriptor_t& descriptor) const;

        system_descriptor_t* find_system_descriptor(const char* name);
        const std::vector<system_descriptor_t>& get_system_descriptors() const;
        bool is_empty() const;

    private:
        std::vector<system_descriptor_t> system_descriptors;
    };
}

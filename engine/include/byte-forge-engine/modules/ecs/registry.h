#pragma once

#include <byte-forge-engine/modules/ecs/ecs_api.h>
#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/ecs/entity.h>
#include <byte-forge-engine/modules/ecs/component_type.h>
#include <byte-forge-engine/modules/ecs/e_system_stages.h>
#include <byte-forge-engine/modules/ecs/system.h>
#include <byte-forge-engine/modules/ecs/system_descriptor.h>

#include <string>
#include <vector>

namespace byteforge::ecs
{
    constexpr usize MAX_COMPONENT_TYPES = 64;

    class BF_ECS_API registry_t
    {
    public:
        std::vector<entity_t> entities;
        std::vector<component_type_t*> component_types;
        std::vector<system_t*> systems;
        std::vector<u64> entities_components_bitsets;
        std::vector<entity_t> entities_to_add;
        std::vector<entity_t> entities_to_kill;
        u64 entities_current_id;

        registry_t() : entities_current_id(0) {}

        registry_t(const registry_t&)            = delete;
        registry_t& operator=(const registry_t&) = delete;

        void dispose();

        void run_systems(const e_system_stages stage, const f64 delta_time);

        entity_t create_entity();
        void publish_entity(const entity_t entity);
        void spawn_entities();
        void spawn_entity(const entity_t entity);
        void add_entity_to_kill_queue(const entity_t entity);
        void kill_entities();
        void kill_entity(const entity_t entity);
        void refresh_entity_systems(const entity_t entity);

        component_type_t* register_component_type(const usize size, const char* name, component_free_function_t free_function);
        component_type_t* get_component_type_by_name(const char* name) const;
        void add_component_to_entity(component_type_t& component_type, const void* const component, const entity_t entity);
        void remove_component_from_entity(component_type_t& component_type, const entity_t entity);

        system_t* create_system(const system_descriptor_t& descriptor, const u64 required_entity_components_bitset);
        bool remove_system(const char* name);
        void sort_systems();

        system_t* find_system(const char* name) const;

        usize validate(std::vector<std::string>& out_issues) const;
        void print_info() const;
        usize get_memory_bytes() const;
    };
}

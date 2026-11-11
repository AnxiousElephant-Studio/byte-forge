#pragma once

#include <byte-forge-engine/modules/ecs/ecs_api.h>
#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/ecs/e_system_stages.h>
#include <byte-forge-engine/modules/ecs/entity.h>

#include <string>
#include <vector>

namespace byteforge::ecs
{
    class registry_t;
    struct system_t;

    using system_update_function_t = void (*)(registry_t& registry, system_t& system, const f64 delta_time);
    using system_free_function_t   = void (*)(void* system_data);

    struct BF_ECS_API system_t
    {
    public:
        u64 id;
        std::string name;
        e_system_stages stage;
        i32 order;
        bool is_enabled;
        void* data;
        u64 required_entity_components_bitset;
        system_update_function_t update_function;
        system_free_function_t free_function;
        std::vector<entity_t> entities;

        system_t(u64 id, const char* name, const e_system_stages stage, const i32 order, void* data, u64 required_entity_components_bitset, system_update_function_t update_function, system_free_function_t free_function)
            : id(id)
            , name(name)
            , stage(stage)
            , order(order)
            , is_enabled(true)
            , data(data)
            , required_entity_components_bitset(required_entity_components_bitset)
            , update_function(update_function)
            , free_function(free_function)
            , entities()
        {
        }

        void dispose();

        void add_entity(const entity_t entity);
        void remove_entity(const entity_t entity);

        bool accepts(const u64 entity_components_bitset) const;
        bool contains_entity(const entity_t entity) const;

        void print_info(const usize registry_index) const;
    };
}

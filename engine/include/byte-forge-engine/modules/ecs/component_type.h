#pragma once

#include <byte-forge-engine/modules/ecs/ecs_api.h>
#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/ecs/entity.h>

#include <string>
#include <unordered_map>
#include <vector>

namespace byteforge::ecs
{
    using component_free_function_t = void (*)(void* component_data);

    struct BF_ECS_API component_type_t
    {
    public:
        u64 id;
        usize size;
        std::string name;
        std::vector<u8> data;
        std::unordered_map<u64, usize> entity_id_to_index;
        component_free_function_t free_function;

        component_type_t(u64 id, usize size, const char* name, component_free_function_t free_function)
            : id(id)
            , size(size)
            , name(name)
            , data()
            , entity_id_to_index()
            , free_function(free_function)
        {
        }

        void dispose();

        void add_or_replace(const entity_t entity, const void* const component);
        void remove(const entity_t entity);

        void* get(const entity_t entity);
        bool has(const entity_t entity) const;

        usize get_count() const;
        usize get_memory_bytes() const;

        void print_info(const usize registry_index) const;
    };
}

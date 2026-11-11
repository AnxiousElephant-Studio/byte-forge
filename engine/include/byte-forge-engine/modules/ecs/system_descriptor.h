#pragma once

#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/ecs/e_system_stages.h>
#include <byte-forge-engine/modules/ecs/system.h>

#include <string>
#include <vector>

namespace byteforge::ecs
{
    struct system_descriptor_t
    {
    public:
        std::string name;
        e_system_stages stage;
        i32 order;
        std::vector<std::string> required_component_type_names;
        system_update_function_t update_function;
        system_free_function_t free_function;
        void* data;

        system_descriptor_t()
            : name()
            , stage(e_system_stages::Update)
            , order(0)
            , required_component_type_names()
            , update_function(nullptr)
            , free_function(nullptr)
            , data(nullptr)
        {
        }

        system_descriptor_t(const char* name, const e_system_stages stage, const i32 order, const std::vector<std::string>& required_component_type_names, system_update_function_t update_function, void* data)
            : name(name)
            , stage(stage)
            , order(order)
            , required_component_type_names(required_component_type_names)
            , update_function(update_function)
            , free_function(nullptr)
            , data(data)
        {
        }
    };
}

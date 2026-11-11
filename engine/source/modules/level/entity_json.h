#pragma once

#include <byte-forge-engine/modules/ecs/entity.h>
#include <byte-forge-engine/modules/scene/scene.h>

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace byteforge::level
{
    using json_t = nlohmann::json;

    struct entity_json_t
    {
    public:
        static json_t serialize(scene::scene_t& scene, const std::vector<ecs::entity_t>& entities);
        static bool deserialize(scene::scene_t& scene, const json_t& entities_json, std::vector<ecs::entity_t>& out_created_entities, std::string& out_error);
    };
}

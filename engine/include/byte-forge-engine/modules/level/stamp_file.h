#pragma once

#include <byte-forge-engine/modules/level/level_api.h>
#include <byte-forge-engine/modules/ecs/entity.h>
#include <byte-forge-engine/modules/scene/scene.h>

#include <string>

namespace byteforge::level
{
    struct BF_LEVEL_API stamp_file_t
    {
    public:
        static constexpr const char* extension = ".bfstamp";

        static bool save(scene::scene_t& scene, const ecs::entity_t root, const char* path, std::string& out_error);
        static bool instantiate(scene::scene_t& scene, const char* path, ecs::entity_t& out_root, std::string& out_error);
    };
}

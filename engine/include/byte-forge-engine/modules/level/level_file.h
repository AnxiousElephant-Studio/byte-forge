#pragma once

#include <byte-forge-engine/modules/level/level_api.h>
#include <byte-forge-engine/modules/scene/scene_context.h>

#include <string>

namespace byteforge::level
{
    struct BF_LEVEL_API level_file_t
    {
    public:
        static constexpr const char* extension = ".bflevel";

        static std::string serialize(scene::scene_context_t& scene_context);

        static bool save(scene::scene_context_t& scene_context, const char* path, std::string& out_error);

        static bool load(scene::scene_context_t& scene_context, const char* path, std::string& out_error);
        static bool deserialize(scene::scene_context_t& scene_context, const std::string& text, std::string& out_error);

        static void clear(scene::scene_context_t& scene_context);
    };
}

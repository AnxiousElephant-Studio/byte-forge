#include <byte-forge-engine/modules/level/level_file.h>
#include <byte-forge-engine/modules/io/file_system.h>
#include <byte-forge-engine/modules/scene/scene_default_2D_systems.h>

#include "entity_json.h"

namespace byteforge::level
{
    namespace
    {
        const int current_format_version = 1;
        const int json_indentation       = 4;

        const char* const version_key         = "format_version";
        const char* const default_systems_key = "default_2D_systems";
        const char* const entities_key        = "entities";

        std::string make_path_error(const char* action, const char* path, const char* cause)
        {
            return std::string("Could not ") + action + " at path: '" + path + "'. Cause: " + cause;
        }
    }

    std::string level_file_t::serialize(scene::scene_context_t& scene_context)
    {
        const json_t json =
        {
            { version_key, current_format_version },
            { default_systems_key, scene::has_any_default_2D_system(scene_context) },
            { entities_key, entity_json_t::serialize(scene_context.scene, scene_context.scene.registry.entities) },
        };

        return json.dump(json_indentation);
    }

    bool level_file_t::save(scene::scene_context_t& scene_context, const char* path, std::string& out_error)
    {
        if (!io::file_system_t::write_text(path, serialize(scene_context)))
        {
            out_error = make_path_error("write the level file", path, "the file could not be written.");
            return false;
        }

        return true;
    }

    bool level_file_t::load(scene::scene_context_t& scene_context, const char* path, std::string& out_error)
    {
        std::string text;

        if (!io::file_system_t::read_text(path, text))
        {
            out_error = make_path_error("read the level file", path, "the file could not be opened.");
            return false;
        }

        std::string cause;

        if (!deserialize(scene_context, text, cause))
        {
            out_error = make_path_error("read the level file", path, cause.c_str());
            return false;
        }

        return true;
    }

    bool level_file_t::deserialize(scene::scene_context_t& scene_context, const std::string& text, std::string& out_error)
    {
        const json_t json = json_t::parse(text, nullptr, false);

        if (json.is_discarded() || !json.is_object() || !json.contains(entities_key))
        {
            out_error = "the text is not a valid level.";
            return false;
        }

        if (json.value(version_key, 0) > current_format_version)
        {
            out_error = "it was saved by a newer version of ByteForge.";
            return false;
        }

        clear(scene_context);

        if (json.value(default_systems_key, false))
        {
            scene::add_default_2D_systems(scene_context);
        }
        else
        {
            scene::remove_default_2D_systems(scene_context);
        }

        std::vector<ecs::entity_t> created_entities;

        return entity_json_t::deserialize(scene_context.scene, json.at(entities_key), created_entities, out_error);
    }

    void level_file_t::clear(scene::scene_context_t& scene_context)
    {
        const std::vector<ecs::entity_t> entities = scene_context.scene.registry.entities;

        for (const ecs::entity_t entity : entities)
        {
            if (scene_context.scene.exists(entity))
            {
                scene_context.scene.destroy_entity(entity);
            }
        }
    }
}

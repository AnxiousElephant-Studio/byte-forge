#include <byte-forge-engine/modules/level/stamp_file.h>
#include <byte-forge-engine/modules/io/file_system.h>

#include "entity_json.h"

#include <unordered_map>

namespace byteforge::level
{
    namespace
    {
        const int current_format_version = 1;
        const int json_indentation       = 4;

        const char* const version_key  = "format_version";
        const char* const entities_key = "entities";

        std::string make_path_error(const char* action, const char* path, const std::string& cause)
        {
            return std::string("Could not ") + action + " at path: '" + path + "'. Cause: " + cause;
        }
    }

    bool stamp_file_t::save(scene::scene_t& scene, const ecs::entity_t root, const char* path, std::string& out_error)
    {
        if (!scene.exists(root))
        {
            out_error = make_path_error("save the stamp", path, "the root entity does not exist.");
            return false;
        }

        std::vector<ecs::entity_t> subtree;
        scene.get_subtree(root, subtree);

        const json_t entities = entity_json_t::serialize(scene, subtree);

        const json_t json =
        {
            { version_key, current_format_version },
            { entities_key, entities },
        };

        if (!io::file_system_t::write_text(path, json.dump(json_indentation)))
        {
            out_error = make_path_error("save the stamp", path, "the file could not be written.");
            return false;
        }

        return true;
    }

    bool stamp_file_t::instantiate(scene::scene_t& scene, const char* path, ecs::entity_t& out_root, std::string& out_error)
    {
        std::string text;

        if (!io::file_system_t::read_text(path, text))
        {
            out_error = make_path_error("instantiate the stamp", path, "the file could not be opened.");
            return false;
        }

        const json_t json = json_t::parse(text, nullptr, false);

        if (json.is_discarded() || !json.is_object() || !json.contains(entities_key) || json.at(entities_key).empty())
        {
            out_error = make_path_error("instantiate the stamp", path, "the file is not a valid stamp.");
            return false;
        }

        std::vector<ecs::entity_t> created_entities;
        std::string cause;

        if (!entity_json_t::deserialize(scene, json.at(entities_key), created_entities, cause))
        {
            out_error = make_path_error("instantiate the stamp", path, cause);
            return false;
        }

        out_root = created_entities.front();

        return true;
    }
}

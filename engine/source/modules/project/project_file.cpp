#include <byte-forge-engine/modules/io/file_system.h>
#include <byte-forge-engine/modules/project/project_file.h>

#include <algorithm>
#include <filesystem>
#include <system_error>

#include <nlohmann/json.hpp>

namespace byteforge::project
{
    namespace
    {
        const int current_format_version = 1;
        const int json_indentation       = 4;

        std::string make_path_error(const char* action, const char* path, const char* cause)
        {
            return std::string("Could not ") + action + " at path: '" + path + "'. Cause: " + cause;
        }

        nlohmann::json to_json(const project_t& project)
        {
            const project_settings_t& settings = project.settings;

            return nlohmann::json
            {
                { "format_version", current_format_version },
                { "name", project.name },
                { "start_level", project.start_level },
                { "settings",
                    {
                        { "target_fps", settings.target_fps },
                        { "use_vsync", settings.use_vsync },
                        { "assets_directory", settings.assets_directory },
                        { "levels_directory", settings.levels_directory },
                        { "sprite_vertex_shader", settings.sprite_vertex_shader },
                        { "sprite_fragment_shader", settings.sprite_fragment_shader },
                    }
                },
            };
        }

        void read_settings(const nlohmann::json& json, project_settings_t& settings)
        {
            settings.target_fps             = std::clamp<u32>(json.value("target_fps", settings.target_fps), 1, 240);
            settings.use_vsync              = json.value("use_vsync", settings.use_vsync);
            settings.assets_directory       = json.value("assets_directory", settings.assets_directory);
            settings.levels_directory       = json.value("levels_directory", settings.levels_directory);
            settings.sprite_vertex_shader   = json.value("sprite_vertex_shader", settings.sprite_vertex_shader);
            settings.sprite_fragment_shader = json.value("sprite_fragment_shader", settings.sprite_fragment_shader);
        }
    }

    bool project_file_t::create(const char* name, const char* location, project_t& out_project, std::string& out_error)
    {
        const std::string root_directory = io::file_system_t::join(location, name);

        std::error_code error;

        if (std::filesystem::exists(root_directory, error) && !std::filesystem::is_empty(root_directory, error))
        {
            out_error = make_path_error("create the project", root_directory.c_str(), "the folder already exists and is not empty.");
            return false;
        }

        project_t project;
        project.name           = name;
        project.root_directory = root_directory;
        project.file_path      = io::file_system_t::join(root_directory.c_str(), (std::string(name) + extension).c_str());

        const std::string assets_directory = io::file_system_t::join(root_directory.c_str(), project.settings.assets_directory.c_str());
        const std::string levels_directory = io::file_system_t::join(root_directory.c_str(), project.settings.levels_directory.c_str());

        if (!io::file_system_t::create_directories(assets_directory.c_str()) || !io::file_system_t::create_directories(levels_directory.c_str()))
        {
            out_error = make_path_error("create the project folders", root_directory.c_str(), "the location could not be written.");
            return false;
        }

        if (!save(project, out_error))
        {
            return false;
        }

        out_project = project;

        return true;
    }

    bool project_file_t::load(const char* path, project_t& out_project, std::string& out_error)
    {
        std::string text;

        if (!io::file_system_t::read_text(path, text))
        {
            out_error = make_path_error("read the project file", path, "the file could not be opened.");
            return false;
        }

        const nlohmann::json json = nlohmann::json::parse(text, nullptr, false);

        if (json.is_discarded() || !json.is_object())
        {
            out_error = make_path_error("read the project file", path, "the file is not valid JSON.");
            return false;
        }

        project_t project;

        try
        {
            if (json.value("format_version", 0) > current_format_version)
            {
                out_error = make_path_error("read the project file", path, "it was saved by a newer version of ByteForge.");
                return false;
            }

            project.name        = json.at("name").get<std::string>();
            project.start_level = json.value("start_level", project.start_level);

            if (json.contains("settings"))
            {
                read_settings(json.at("settings"), project.settings);
            }
        }
        catch (const nlohmann::json::exception& exception)
        {
            out_error = make_path_error("read the project file", path, exception.what());
            return false;
        }

        std::error_code error;
        const std::filesystem::path absolute_path = std::filesystem::absolute(path, error);

        project.file_path      = absolute_path.generic_string();
        project.root_directory = absolute_path.parent_path().generic_string();

        out_project = project;

        return true;
    }

    bool project_file_t::save(const project_t& project, std::string& out_error)
    {
        if (!io::file_system_t::write_text(project.file_path.c_str(), to_json(project).dump(json_indentation)))
        {
            out_error = make_path_error("write the project file", project.file_path.c_str(), "the file could not be written.");
            return false;
        }

        return true;
    }
}

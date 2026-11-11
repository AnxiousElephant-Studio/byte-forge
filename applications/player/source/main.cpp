#include "player_application.h"

#include <byte-forge-engine/modules/io/file_system.h>
#include <byte-forge-engine/modules/logging/log_macros.h>
#include <byte-forge-engine/modules/project/project_file.h>

#include <cstdlib>
#include <string>
#include <vector>

namespace
{
    const char* const missing_project_error = "Could not find a project. Pass the path of a %s file, or run the player inside the project folder.";

    std::string find_project_file()
    {
        std::vector<byteforge::io::directory_entry_t> entries;
        byteforge::io::file_system_t::list_directory(".", entries);

        for (const byteforge::io::directory_entry_t& entry : entries)
        {
            if (!entry.is_directory && entry.extension == byteforge::project::project_file_t::extension)
            {
                return entry.path;
            }
        }

        return std::string();
    }
}

int main(const int argument_count, const char** arguments)
{
    const std::string project_file_path = argument_count > 1 ? std::string(arguments[1]) : find_project_file();

    if (project_file_path.empty())
    {
        LOG_FATAL(missing_project_error, byteforge::project::project_file_t::extension);
        return EXIT_FAILURE;
    }

    byteforge::player::player_application_t player_application;

    if (!player_application.initialize(project_file_path))
    {
        player_application.dispose();
        return EXIT_FAILURE;
    }

    player_application.run();
    player_application.dispose();

    return EXIT_SUCCESS;
}

#pragma once

#include <byte-forge-engine/modules/project/project_settings.h>

#include <string>

namespace byteforge::project
{
    struct project_t
    {
    public:
        std::string name;
        std::string file_path;
        std::string root_directory;
        std::string start_level;
        project_settings_t settings;
    };
}

#pragma once

#include <byte-forge-engine/modules/project/project_api.h>
#include <byte-forge-engine/modules/project/project.h>

#include <string>

namespace byteforge::project
{
    struct BF_PROJECT_API project_file_t
    {
    public:
        static constexpr const char* extension = ".bfproject";

        static bool create(const char* name, const char* location, project_t& out_project, std::string& out_error);
        static bool load(const char* path, project_t& out_project, std::string& out_error);
        static bool save(const project_t& project, std::string& out_error);
    };
}

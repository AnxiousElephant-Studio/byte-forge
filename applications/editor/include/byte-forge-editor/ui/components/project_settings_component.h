#pragma once

#include <byte-forge-editor/editor_application_context.h>

#include <string>

namespace byteforge::editor
{
    class project_settings_component_t
    {
    public:
        void draw(editor_application_context_t& context);

    private:
        void load_from_project(const project::project_t& project);
        void save_to_project(editor_application_context_t& context);

        bool is_loaded = false;
        int target_fps = 0;
        bool use_vsync = false;

        char name_buffer[128]                   = {};
        char assets_directory_buffer[256]       = {};
        char levels_directory_buffer[256]       = {};
        char sprite_vertex_shader_buffer[256]   = {};
        char sprite_fragment_shader_buffer[256] = {};

        std::string status_message;
        bool is_status_error = false;
    };
}

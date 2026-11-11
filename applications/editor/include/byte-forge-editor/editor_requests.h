#pragma once

#include <byte-forge-engine/modules/ecs/entity.h>

#include <string>

namespace byteforge::editor
{
    struct editor_requests_t
    {
    public:
        std::string file_to_edit;
        std::string project_file_to_open;
        std::string project_open_error;
        bool is_sprite_shader_reload_requested;
        bool is_project_settings_window_open;
        bool is_project_settings_apply_requested;

        std::string level_file_to_open;
        std::string level_name_to_create;
        std::string level_error;
        bool is_level_save_requested = false;
        bool is_start_level_change_requested = false;

        bool is_play_requested = false;
        bool is_stop_requested = false;

        std::string stamp_to_instantiate;
        ecs::entity_t stamp_root_to_save;
        bool is_stamp_save_requested = false;
        std::string stamp_error;

        editor_requests_t()
            : file_to_edit()
            , project_file_to_open()
            , project_open_error()
            , is_sprite_shader_reload_requested(false)
            , is_project_settings_window_open(false)
            , is_project_settings_apply_requested(false)
        {
        }
    };
}

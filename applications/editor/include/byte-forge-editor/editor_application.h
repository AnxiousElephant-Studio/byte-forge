#pragma once

#include <byte-forge-editor/editor_application_context.h>
#include <byte-forge-editor/ui/views/main_view/main_view.h>
#include <byte-forge-editor/ui/views/project_creation_view/project_creation_view.h>

#include <byte-forge-engine/modules/platform/display/window/window_event_callback.h>
#include <byte-forge-engine/modules/platform/input/input_event_callback.h>
#include <byte-forge-engine/modules/runtime/engine_runtime.h>

namespace byteforge::editor
{
    class editor_application_t
    {
    public:
        editor_application_t();

        bool initialize();
        void dispose();

        void run();
        void request_project(const std::string& project_file_path);

    private:
        bool create_main_window();
        void bind_platform_callbacks();

        void poll_events();
        void update();
        void update_scene();
        void draw_ui();
        void render_ui();
        void process_project_request();
        void apply_project_settings();

        void process_level_requests();
        void open_start_level();
        bool open_level(const std::string& level_path);
        bool create_level(const std::string& level_name);
        void save_level();
        void set_current_level(const std::string& level_path);
        void update_level_dirty_state();
        void reload_modified_assets();

        void process_play_requests();
        void start_playing();
        void stop_playing();
        void process_script_level_request();
        void process_stamp_requests();
        void save_stamp(const ecs::entity_t root);
        void instantiate_stamp(const std::string& stamp_path);

        runtime::engine_runtime_t engine_runtime;
        editor_application_context_t context;
        main_view_t main_view;
        project_creation_view_t project_creation_view;
        unsigned int frames_since_dirty_check = 0;
        unsigned int frames_since_asset_reload = 0;
        std::string play_snapshot_text;

        platform::input_event_callback_t input_event_callback;
        platform::window_event_callback_t window_event_callback;
    };
}

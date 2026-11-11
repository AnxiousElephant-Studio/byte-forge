#pragma once

#include <byte-forge-engine/modules/platform/display/window/window.h>
#include <byte-forge-engine/modules/platform/input/input_event_callback.h>
#include <byte-forge-engine/modules/platform/display/window/window_event_callback.h>
#include <byte-forge-engine/modules/project/project.h>
#include <byte-forge-engine/modules/rendering/lighting/point_light_renderer.h>
#include <byte-forge-engine/modules/rendering/opengl/framebuffer/opengl_framebuffer.h>
#include <byte-forge-engine/modules/rendering/sprite/sprite_renderer.h>
#include <byte-forge-engine/modules/runtime/engine_runtime.h>
#include <byte-forge-engine/modules/scene/scene_context.h>

#include <string>

namespace byteforge::player
{
    class player_application_t
    {
    public:
        player_application_t();

        bool initialize(const std::string& project_file_path);
        void run();
        void dispose();

    private:
        bool load_project(const std::string& project_file_path);
        bool create_window();
        bool initialize_scene();
        void bind_platform_callbacks();
        void process_level_request();

        project::project_t project;
        runtime::engine_runtime_t engine_runtime;
        platform::window_t* window;
        scene::scene_context_t scene_context;
        rendering::sprite_renderer_t sprite_renderer;
        rendering::point_light_renderer_t point_light_renderer;
        rendering::opengl_framebuffer_t light_map_framebuffer;
        platform::input_event_callback_t input_event_callback;
        platform::window_event_callback_t window_event_callback;
    };
}

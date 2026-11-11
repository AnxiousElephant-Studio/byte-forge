#include "player_application.h"

#include <byte-forge-engine/modules/level/level_file.h>
#include <byte-forge-engine/modules/level/stamp_file.h>
#include <byte-forge-engine/modules/logging/log_macros.h>
#include <byte-forge-engine/modules/platform/display/window/window_descriptor.h>
#include <byte-forge-engine/modules/platform/display/window/window_event.h>
#include <byte-forge-engine/modules/project/project_file.h>
#include <byte-forge-engine/modules/rendering/sprite/sprite_shader_descriptor.h>

#include <filesystem>
#include <system_error>

namespace byteforge::player
{
    namespace
    {
        const math::recti_t window_bounds(0, 0, 1280, 720);

        const char* const project_error_format      = "Could not open the project. Cause: %s";
        const char* const runtime_error_format      = "Could not start the engine runtime.";
        const char* const window_error_format       = "Could not create the game window.";
        const char* const scene_error_format        = "Could not initialize the scene.";
        const char* const sprite_error_format       = "Could not load the sprite shaders: '%s' and '%s'.";
        const char* const lighting_warning_format   = "Could not initialize the point light renderer. Point lights will not be drawn.";
        const char* const missing_level_error       = "Could not start the game. Cause: the project has no start level.";
        const char* const level_error_format        = "%s";
    }

    player_application_t::player_application_t()
        : project()
        , engine_runtime(platform::display_descriptor_t(graphics::e_graphic_backends::OpenGL), time::frame_settings_t(60))
        , window(nullptr)
        , scene_context()
        , sprite_renderer()
        , point_light_renderer()
        , light_map_framebuffer()
        , input_event_callback()
        , window_event_callback()
    {
    }

    bool player_application_t::initialize(const std::string& project_file_path)
    {
        if (!load_project(project_file_path))
        {
            return false;
        }

        engine_runtime.frame_settings = time::frame_settings_t(static_cast<u8>(project.settings.target_fps));

        if (!engine_runtime.initialize())
        {
            LOG_FATAL(runtime_error_format);
            return false;
        }

        if (!create_window() || !initialize_scene())
        {
            return false;
        }

        bind_platform_callbacks();

        if (project.start_level.empty())
        {
            LOG_FATAL(missing_level_error);
            return false;
        }

        std::string error;

        if (!level::level_file_t::load(scene_context, project.start_level.c_str(), error))
        {
            LOG_FATAL(level_error_format, error.c_str());
            return false;
        }

        scene_context.scene_player.start();

        return true;
    }

    void player_application_t::run()
    {
        platform::input_state_t& input_state = engine_runtime.platform.context.input_state;

        while (!engine_runtime.should_close)
        {
            engine_runtime.frame_manager.start_frame();

            input_state.begin_frame();
            engine_runtime.platform.poll_events(input_event_callback, window_event_callback);

            if (engine_runtime.renderer.begin_frame(*window))
            {
                scene_context.set_render_target_size(window->get_size());
                scene_context.update(engine_runtime.frame_manager.delta_time);

                engine_runtime.renderer.end_frame(*window);
            }

            process_level_request();

            engine_runtime.audio.update();
            engine_runtime.frame_manager.end_frame();
        }
    }

    void player_application_t::dispose()
    {
        scene_context.dispose();
        light_map_framebuffer.dispose();
        point_light_renderer.dispose();
        sprite_renderer.dispose();
        engine_runtime.dispose();
    }

    bool player_application_t::load_project(const std::string& project_file_path)
    {
        std::string error;

        if (!project::project_file_t::load(project_file_path.c_str(), project, error))
        {
            LOG_FATAL(project_error_format, error.c_str());
            return false;
        }

        std::error_code directory_error;
        std::filesystem::current_path(project.root_directory, directory_error);

        if (directory_error)
        {
            LOG_FATAL(project_error_format, directory_error.message().c_str());
            return false;
        }

        return true;
    }

    bool player_application_t::create_window()
    {
        const platform::window_descriptor_t descriptor(project.name.c_str(), window_bounds, platform::e_window_flags::Resizable, project.settings.use_vsync);
        window = engine_runtime.create_window(descriptor);

        if (window == nullptr || !engine_runtime.renderer.begin_frame(*window))
        {
            LOG_FATAL(window_error_format);
            return false;
        }

        engine_runtime.platform.set_current_window(*window);

        return true;
    }

    bool player_application_t::initialize_scene()
    {
        platform::input_state_t& input_state = engine_runtime.platform.context.input_state;

        if (!scene_context.initialize(engine_runtime.scripting_context, engine_runtime.audio, input_state))
        {
            LOG_FATAL(scene_error_format);
            return false;
        }

        const project::project_settings_t& settings = project.settings;

        if (!sprite_renderer.initialize(rendering::sprite_shader_descriptor_t(settings.sprite_vertex_shader.c_str(), settings.sprite_fragment_shader.c_str())))
        {
            LOG_FATAL(sprite_error_format, settings.sprite_vertex_shader.c_str(), settings.sprite_fragment_shader.c_str());
            return false;
        }

        scene_context.set_render_resources(sprite_renderer, engine_runtime.asset_manager);
        scene_context.set_render_target_framebuffer(nullptr);
        scene_context.scene_player.script_system.stamp_loader = level::stamp_file_t::instantiate;

        if (point_light_renderer.initialize())
        {
            scene_context.set_lighting_resources(point_light_renderer, light_map_framebuffer);
        }
        else
        {
            LOG_WARNING(lighting_warning_format);
        }

        input_state.set_mouse_target(math::vec2_t(), math::vec2_t(1.0f, 1.0f));

        return true;
    }

    void player_application_t::bind_platform_callbacks()
    {
        input_event_callback.callback = [this](const platform::input_event_t& event)
        {
            engine_runtime.platform.context.input_state.process(event);
        };

        window_event_callback.callback = [this](const platform::window_event_t& event)
        {
            if (event.type == platform::e_window_event_types::CloseRequested)
            {
                engine_runtime.platform.request_close();
            }
        };
    }

    void player_application_t::process_level_request()
    {
        scene::scene_player_t& scene_player = scene_context.scene_player;
        std::string level_path;

        if (!scene_player.script_system.take_level_request(level_path))
        {
            return;
        }

        scene_player.stop();

        std::string error;

        if (!level::level_file_t::load(scene_context, level_path.c_str(), error))
        {
            LOG_ERROR(level_error_format, error.c_str());
        }

        scene_player.start();
    }
}

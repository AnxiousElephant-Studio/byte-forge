#include <byte-forge-editor/editor_application.h>
#include <byte-forge-editor/editor_application_settings.h>
#include <byte-forge-editor/ui/utils/file_name_validation.h>

#include <byte-forge-engine/modules/io/file_system.h>
#include <byte-forge-engine/modules/level/level_file.h>
#include <byte-forge-engine/modules/level/stamp_file.h>
#include <byte-forge-engine/modules/logging/log_macros.h>
#include <byte-forge-engine/modules/platform/display/window/window_descriptor.h>
#include <byte-forge-engine/modules/platform/display/window/window_event.h>
#include <byte-forge-engine/modules/project/project_file.h>
#include <byte-forge-engine/modules/rendering/opengl/pass/opengl_render_pass.h>
#include <byte-forge-engine/modules/rendering/sprite/sprite_shader_descriptor.h>

#include <filesystem>
#include <string>
#include <system_error>

namespace byteforge::editor
{
    using settings_t = editor_application_settings_t;

    editor_application_t::editor_application_t()
        : engine_runtime(platform::display_descriptor_t(settings_t::graphic_backend), time::frame_settings_t(static_cast<u8>(project::project_settings_t().target_fps)))
        , context()
        , main_view()
        , input_event_callback()
        , window_event_callback()
    {
    }

    bool editor_application_t::initialize()
    {
        if (!engine_runtime.initialize())
        {
            LOG_FATAL(settings_t::engine_runtime_startup_error_format);
            return false;
        }

        if (!create_main_window())
        {
            return false;
        }

        if (!context.scene_context.initialize(engine_runtime.scripting_context, engine_runtime.audio, engine_runtime.platform.context.input_state))
        {
            LOG_FATAL(settings_t::scene_error_format);
            return false;
        }

        std::error_code directory_error;
        context.editor_directory = std::filesystem::current_path(directory_error).generic_string();

        if (!context.debug_ui_context.initialize(*engine_runtime.platform.get_current_window()))
        {
            return false;
        }

        context.engine_runtime                      = &engine_runtime;
        context.scene_context.scene_player.profiler = &context.profiler;
        context.scene_context.set_render_resources(context.sprite_renderer, engine_runtime.asset_manager);
        context.scene_context.set_render_target_framebuffer(&context.scene_framebuffer);
        context.scene_context.scene_player.script_system.stamp_loader = level::stamp_file_t::instantiate;

        if (context.point_light_renderer.initialize())
        {
            context.scene_context.set_lighting_resources(context.point_light_renderer, context.light_map_framebuffer);
        }
        else
        {
            LOG_ERROR(settings_t::point_light_renderer_error_format);
        }

        bind_platform_callbacks();

        return true;
    }

    void editor_application_t::dispose()
    {
        context.scene_context.dispose();
        context.scene_framebuffer.dispose();
        context.light_map_framebuffer.dispose();
        context.point_light_renderer.dispose();
        context.sprite_renderer.dispose();
        context.debug_ui_context.dispose();
        engine_runtime.dispose();
    }

    void editor_application_t::run()
    {
        while (!engine_runtime.should_close)
        {
            context.profiler.begin(settings_t::frame_profile_name);
            engine_runtime.frame_manager.start_frame();

            poll_events();

            platform::window_t& window = *engine_runtime.platform.get_current_window();

            if (engine_runtime.renderer.begin_frame(window))
            {
                draw_ui();
                update();
                render_ui();

                engine_runtime.renderer.end_frame(window);
            }

            engine_runtime.frame_manager.end_frame();
            context.profiler.end(settings_t::frame_profile_name);
        }
    }

    bool editor_application_t::create_main_window()
    {
        const platform::window_descriptor_t descriptor(settings_t::window_title, settings_t::window_bounds, settings_t::window_flags, project::project_settings_t().use_vsync);
        platform::window_t* window = engine_runtime.create_window(descriptor);

        if (window == nullptr)
        {
            LOG_FATAL(settings_t::window_error_format, settings_t::window_title);
            return false;
        }

        if (!engine_runtime.renderer.begin_frame(*window))
        {
            LOG_FATAL(settings_t::renderer_begin_frame_error_format, settings_t::window_title);
            return false;
        }

        engine_runtime.platform.set_current_window(*window);

        return true;
    }

    void editor_application_t::bind_platform_callbacks()
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

        engine_runtime.platform.native_event_callback.callback = [this](const void* native_event)
        {
            context.debug_ui_context.process_event(native_event);
        };
    }

    void editor_application_t::poll_events()
    {
        const time::scoped_profile_t events_profile(&context.profiler, settings_t::events_profile_name);

        engine_runtime.platform.context.input_state.begin_frame();
        engine_runtime.platform.poll_events(input_event_callback, window_event_callback);
    }

    void editor_application_t::update()
    {
        const time::scoped_profile_t update_profile(&context.profiler, settings_t::update_profile_name);

        if (context.has_open_project)
        {
            process_play_requests();
            update_scene();
            process_script_level_request();
        }

        engine_runtime.audio.update();
    }

    void editor_application_t::update_scene()
    {
        rendering::opengl_framebuffer_t& framebuffer = context.scene_framebuffer;
        const math::vec2i_t scene_size               = context.scene_viewport_size;

        if (!framebuffer.resize(scene_size))
        {
            return;
        }

        rendering::opengl_render_pass_t::begin(framebuffer, engine_runtime.renderer.clear_color);

        context.scene_context.set_render_target_size(scene_size);
        context.scene_context.update(engine_runtime.frame_manager.delta_time);

        rendering::opengl_render_pass_t::begin_window(engine_runtime.platform.get_current_window()->get_size());
    }

    void editor_application_t::draw_ui()
    {
        const time::scoped_profile_t ui_profile(&context.profiler, settings_t::ui_profile_name);

        context.debug_ui_context.begin_frame();

        if (context.has_open_project)
        {
            main_view.draw(context);
        }
        else
        {
            project_creation_view.draw(context);
        }
    }

    void editor_application_t::render_ui()
    {
        const time::scoped_profile_t ui_render_profile(&context.profiler, settings_t::ui_render_profile_name);

        context.debug_ui_context.end_frame();

        process_project_request();
        process_level_requests();
        process_stamp_requests();

        if (context.requests.is_project_settings_apply_requested)
        {
            context.requests.is_project_settings_apply_requested = false;
            apply_project_settings();
        }
    }

    void editor_application_t::process_project_request()
    {
        if (context.requests.project_file_to_open.empty())
        {
            return;
        }

        const std::string project_file_path = context.requests.project_file_to_open;
        context.requests.project_file_to_open.clear();

        project::project_t project;

        if (!project::project_file_t::load(project_file_path.c_str(), project, context.requests.project_open_error))
        {
            LOG_ERROR("%s", context.requests.project_open_error.c_str());
            return;
        }

        std::error_code directory_error;
        std::filesystem::current_path(project.root_directory, directory_error);

        if (directory_error || !io::file_system_t::create_directories(project.settings.assets_directory.c_str()))
        {
            context.requests.project_open_error = "Could not open the project folder at path: '" + project.root_directory + "'.";
            LOG_ERROR(settings_t::assets_error_format, project.settings.assets_directory.c_str());
            return;
        }

        const platform::window_t& window = *engine_runtime.platform.get_current_window();

        window.set_resizable(true);
        window.set_size(settings_t::editor_window_size);
        window.center();
        window.maximize();

        context.assets_directory = project.settings.assets_directory;
        context.current_project  = project;
        context.has_open_project = true;

        apply_project_settings();
        open_start_level();
    }

    void editor_application_t::process_level_requests()
    {
        editor_requests_t& requests = context.requests;

        if (requests.is_level_save_requested)
        {
            requests.is_level_save_requested = false;
            save_level();
        }

        if (requests.is_start_level_change_requested)
        {
            requests.is_start_level_change_requested = false;
            context.current_project.start_level      = context.current_level_path;

            if (!project::project_file_t::save(context.current_project, requests.level_error))
            {
                LOG_ERROR("%s", requests.level_error.c_str());
            }
        }

        if (!requests.level_file_to_open.empty())
        {
            const std::string level_path = requests.level_file_to_open;
            requests.level_file_to_open.clear();
            open_level(level_path);
        }

        if (!requests.level_name_to_create.empty())
        {
            const std::string level_name = requests.level_name_to_create;
            requests.level_name_to_create.clear();
            create_level(level_name);
        }

        update_level_dirty_state();
        reload_modified_assets();
    }

    void editor_application_t::open_start_level()
    {
        const std::string start_level = context.current_project.start_level;

        if (!start_level.empty())
        {
            open_level(start_level);
            return;
        }

        if (create_level(settings_t::default_level_name))
        {
            context.current_project.start_level = context.current_level_path;

            if (!project::project_file_t::save(context.current_project, context.requests.level_error))
            {
                LOG_ERROR("%s", context.requests.level_error.c_str());
            }
        }
    }

    bool editor_application_t::open_level(const std::string& level_path)
    {
        scene::scene_context_t& scene_context = context.scene_context;

        stop_playing();

        if (!level::level_file_t::load(scene_context, level_path.c_str(), context.requests.level_error))
        {
            LOG_ERROR("%s", context.requests.level_error.c_str());
            return false;
        }

        set_current_level(level_path);

        return true;
    }

    bool editor_application_t::create_level(const std::string& level_name)
    {
        const std::string levels_directory = context.current_project.settings.levels_directory;
        const std::string level_path       = io::file_system_t::join(levels_directory.c_str(), (level_name + level::level_file_t::extension).c_str());

        if (io::file_system_t::exists(level_path.c_str()))
        {
            context.requests.level_error = "Could not create the level at path: '" + level_path + "'. Cause: the file already exists.";
            return false;
        }

        if (!io::file_system_t::create_directories(levels_directory.c_str()))
        {
            context.requests.level_error = "Could not create the levels folder at path: '" + levels_directory + "'.";
            return false;
        }

        scene::scene_context_t& scene_context = context.scene_context;

        stop_playing();

        level::level_file_t::clear(scene_context);

        if (!level::level_file_t::save(scene_context, level_path.c_str(), context.requests.level_error))
        {
            LOG_ERROR("%s", context.requests.level_error.c_str());
            context.current_level_path.clear();
            context.current_level_name.clear();
            return false;
        }

        set_current_level(level_path);

        return true;
    }

    void editor_application_t::save_level()
    {
        if (context.current_level_path.empty() || context.scene_context.scene_player.is_running())
        {
            return;
        }

        if (!level::level_file_t::save(context.scene_context, context.current_level_path.c_str(), context.requests.level_error))
        {
            LOG_ERROR("%s", context.requests.level_error.c_str());
            return;
        }

        context.saved_level_text = level::level_file_t::serialize(context.scene_context);
        context.is_level_dirty   = false;
    }

    void editor_application_t::set_current_level(const std::string& level_path)
    {
        const std::string file_name = io::file_system_t::get_file_name(level_path.c_str());
        const std::string extension = io::file_system_t::get_extension(level_path.c_str());

        context.current_level_path = level_path;
        context.current_level_name = file_name.substr(0, file_name.size() - extension.size());
        context.saved_level_text   = level::level_file_t::serialize(context.scene_context);
        context.is_level_dirty     = false;

        context.selection.clear();
    }

    void editor_application_t::reload_modified_assets()
    {
        frames_since_asset_reload++;

        if (frames_since_asset_reload < settings_t::asset_reload_interval_in_frames)
        {
            return;
        }

        frames_since_asset_reload = 0;
        engine_runtime.asset_manager.material_cache.reload_modified();
    }

    void editor_application_t::update_level_dirty_state()
    {
        frames_since_dirty_check++;

        if (frames_since_dirty_check < settings_t::level_dirty_check_interval_in_frames)
        {
            return;
        }

        frames_since_dirty_check = 0;

        if (context.has_open_project && !context.current_level_path.empty() && !context.scene_context.scene_player.is_running())
        {
            context.is_level_dirty = level::level_file_t::serialize(context.scene_context) != context.saved_level_text;
        }
    }

    void editor_application_t::apply_project_settings()
    {
        const project::project_settings_t& settings = context.current_project.settings;

        platform::window_t& window = *engine_runtime.platform.get_current_window();

        const std::string window_title = settings_t::project_window_title_prefix + context.current_project.name;
        window.set_title(window_title.c_str());

        window.descriptor.use_vsync = settings.use_vsync;
        engine_runtime.renderer.set_vsync(window, settings.use_vsync);

        engine_runtime.frame_settings = time::frame_settings_t(static_cast<u8>(settings.target_fps));
        engine_runtime.frame_manager.reset(engine_runtime.frame_settings);

        if (!context.sprite_renderer.is_initialized())
        {
            context.sprite_renderer.initialize(rendering::sprite_shader_descriptor_t(settings.sprite_vertex_shader.c_str(), settings.sprite_fragment_shader.c_str()));
        }
    }

    void editor_application_t::process_play_requests()
    {
        editor_requests_t& requests = context.requests;

        if (requests.is_stop_requested)
        {
            requests.is_stop_requested = false;
            stop_playing();
        }

        if (requests.is_play_requested)
        {
            requests.is_play_requested = false;
            start_playing();
        }
    }

    void editor_application_t::start_playing()
    {
        scene::scene_player_t& scene_player = context.scene_context.scene_player;

        if (scene_player.is_running())
        {
            return;
        }

        play_snapshot_text = level::level_file_t::serialize(context.scene_context);
        scene_player.start();
    }

    void editor_application_t::stop_playing()
    {
        scene::scene_player_t& scene_player = context.scene_context.scene_player;

        if (!scene_player.is_running())
        {
            return;
        }

        scene_player.stop();
        context.selection.clear();

        std::string error;

        if (!play_snapshot_text.empty() && !level::level_file_t::deserialize(context.scene_context, play_snapshot_text, error))
        {
            LOG_ERROR(settings_t::play_restore_error_format, error.c_str());
        }

        play_snapshot_text.clear();
    }

    void editor_application_t::process_script_level_request()
    {
        scene::scene_player_t& scene_player = context.scene_context.scene_player;
        std::string level_path;

        if (!scene_player.is_running() || !scene_player.script_system.take_level_request(level_path))
        {
            return;
        }

        scene_player.stop();
        context.selection.clear();

        std::string error;

        if (!level::level_file_t::load(context.scene_context, level_path.c_str(), error))
        {
            LOG_ERROR("%s", error.c_str());
        }

        scene_player.start();
    }

    void editor_application_t::process_stamp_requests()
    {
        editor_requests_t& requests = context.requests;

        if (requests.is_stamp_save_requested)
        {
            requests.is_stamp_save_requested = false;
            save_stamp(requests.stamp_root_to_save);
        }

        if (!requests.stamp_to_instantiate.empty())
        {
            const std::string stamp_path = requests.stamp_to_instantiate;
            requests.stamp_to_instantiate.clear();
            instantiate_stamp(stamp_path);
        }
    }

    void editor_application_t::save_stamp(const ecs::entity_t root)
    {
        scene::scene_t& scene = context.scene_context.scene;
        const components::name_component_t* name = scene.get_name(root);

        if (name == nullptr)
        {
            return;
        }

        std::string file_name = name->name;

        for (char& character : file_name)
        {
            if (has_invalid_file_name_characters(std::string(1, character).c_str()) || character == ' ')
            {
                character = '_';
            }
        }

        const std::string stamps_directory = io::file_system_t::join(context.assets_directory.c_str(), settings_t::stamps_directory_name);

        if (!io::file_system_t::create_directories(stamps_directory.c_str()))
        {
            context.requests.stamp_error = "Could not create the stamps folder at path: '" + stamps_directory + "'.";
            return;
        }

        const std::string stamp_path = io::file_system_t::join(stamps_directory.c_str(), (file_name + level::stamp_file_t::extension).c_str());

        if (!level::stamp_file_t::save(scene, root, stamp_path.c_str(), context.requests.stamp_error))
        {
            LOG_ERROR("%s", context.requests.stamp_error.c_str());
            return;
        }

        context.requests.stamp_error.clear();
        LOG_INFO(settings_t::stamp_saved_format, stamp_path.c_str());
    }

    void editor_application_t::instantiate_stamp(const std::string& stamp_path)
    {
        ecs::entity_t root;

        if (!level::stamp_file_t::instantiate(context.scene_context.scene, stamp_path.c_str(), root, context.requests.stamp_error))
        {
            LOG_ERROR("%s", context.requests.stamp_error.c_str());
            return;
        }

        context.requests.stamp_error.clear();
        context.selection.select(root);
    }

    void editor_application_t::request_project(const std::string& project_file_path)
    {
        context.requests.project_file_to_open = project_file_path;
    }
}

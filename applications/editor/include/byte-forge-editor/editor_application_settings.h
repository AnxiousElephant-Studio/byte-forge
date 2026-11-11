#pragma once

#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/graphics/e_graphic_backends.h>
#include <byte-forge-engine/modules/math/rects/recti.h>
#include <byte-forge-engine/modules/platform/display/window/e_window_flags.h>

namespace byteforge::editor
{
    struct editor_application_settings_t
    {
        static constexpr graphics::e_graphic_backends graphic_backend = graphics::e_graphic_backends::OpenGL;

        static constexpr const char* window_title              = "ByteForge - Studio | Project Creation";
        static constexpr const char* project_window_title_prefix = "ByteForge - Studio | ";
        static constexpr math::recti_t window_bounds           = math::recti_t(0, 0, 900, 700);
        static constexpr platform::e_window_flags window_flags = platform::e_window_flags::None;
        static constexpr math::vec2i_t editor_window_size      = math::vec2i_t(1600, 900);

        static constexpr const char* default_level_name                   = "Main";
        static constexpr unsigned int level_dirty_check_interval_in_frames = 30;
        static constexpr unsigned int asset_reload_interval_in_frames      = 30;

        static constexpr const char* template_shaders_directory = "assets/shaders";
        static constexpr const char* project_shaders_directory  = "assets/shaders";

        static constexpr const char* frame_profile_name  = "frame";
        static constexpr const char* events_profile_name = "events";
        static constexpr const char* update_profile_name = "update";
        static constexpr const char* ui_profile_name     = "ui";
        static constexpr const char* ui_render_profile_name = "ui_render";

        static constexpr const char* engine_runtime_startup_error_format = "Could not start the editor. Cause: the engine runtime failed to initialize.";
        static constexpr const char* window_error_format                 = "Could not create the editor window with title: '%s'.";
        static constexpr const char* renderer_begin_frame_error_format   = "Could not begin the first frame of the editor window with title: '%s'. Cause: its graphics context could not be made current.";
        static constexpr const char* scene_error_format                  = "Could not initialize the scene context. Cause: the scene or the scene player failed to initialize.";
        static constexpr const char* play_restore_error_format           = "Could not restore the scene after playing. Cause: %s";
        static constexpr const char* stamp_saved_format                 = "Saved stamp at path: '%s'.";
        static constexpr const char* stamps_directory_name              = "stamps";
        static constexpr const char* point_light_renderer_error_format   = "Could not initialize the point light renderer. Cause: the light shader failed to compile. Point lights will not be drawn.";
        static constexpr const char* assets_error_format                 = "Could not create the assets directory at path: '%s'.";
    };
}

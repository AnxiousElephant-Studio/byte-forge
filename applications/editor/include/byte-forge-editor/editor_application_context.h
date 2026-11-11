#pragma once

#include <byte-forge-editor/editor_requests.h>
#include <byte-forge-editor/editor_selection.h>
#include <byte-forge-editor/ui/debug_ui_context.h>

#include <byte-forge-engine/modules/project/project.h>
#include <byte-forge-engine/modules/rendering/lighting/point_light_renderer.h>
#include <byte-forge-engine/modules/rendering/opengl/framebuffer/opengl_framebuffer.h>
#include <byte-forge-engine/modules/rendering/sprite/sprite_renderer.h>
#include <byte-forge-engine/modules/runtime/engine_runtime.h>
#include <byte-forge-engine/modules/scene/scene_context.h>
#include <byte-forge-engine/modules/time/profiler.h>

#include <string>

namespace byteforge::editor
{
    struct editor_application_context_t
    {
    public:
        runtime::engine_runtime_t* engine_runtime;
        scene::scene_context_t scene_context;
        rendering::sprite_renderer_t sprite_renderer;
        debug_ui_context_t debug_ui_context;
        time::profiler_t profiler;

        editor_selection_t selection;
        editor_requests_t requests;
        std::string assets_directory;
        std::string editor_directory;

        project::project_t current_project;
        bool has_open_project;

        std::string current_level_path;
        std::string current_level_name;
        std::string saved_level_text;
        bool is_level_dirty = false;

        rendering::opengl_framebuffer_t scene_framebuffer = rendering::opengl_framebuffer_t(rendering::opengl_framebuffer_descriptor_t(false, true));
        math::vec2i_t scene_viewport_size = math::vec2i_t(1, 1);

        rendering::point_light_renderer_t point_light_renderer;
        rendering::opengl_framebuffer_t light_map_framebuffer;

        editor_application_context_t()
            : engine_runtime(nullptr)
            , scene_context()
            , sprite_renderer()
            , debug_ui_context()
            , profiler()
            , selection()
            , requests()
            , assets_directory()
            , editor_directory()
            , current_project()
            , has_open_project(false)
        {
        }
    };
}

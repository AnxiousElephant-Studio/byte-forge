#pragma once

#include <byte-forge-editor/editor_application_context.h>
#include <byte-forge-editor/ui/components/main_menu_bar_component.h>
#include <byte-forge-editor/ui/components/content_browser_component.h>
#include <byte-forge-editor/ui/components/scene_hierarchy_component.h>
#include <byte-forge-editor/ui/components/object_inspector_component.h>
#include <byte-forge-editor/ui/components/project_settings_component.h>
#include <byte-forge-editor/ui/components/viewport_component.h>
#include <byte-forge-editor/ui/components/code_editor_component.h>
#include <byte-forge-editor/ui/components/sprite_renderer_component.h>
#include <byte-forge-editor/ui/components/console_output_component.h>
#include <byte-forge-editor/ui/components/status_panel_component.h>
#include <byte-forge-editor/ui/components/ecs_observer_component.h>

namespace byteforge::editor
{
    class main_view_t
    {
    public:
        void draw(editor_application_context_t& context);

    private:
        main_menu_bar_component_t main_menu_bar_component;
        content_browser_component_t content_browser_component;
        scene_hierarchy_component_t scene_hierarchy_component;
        object_inspector_component_t object_inspector_component;
        code_editor_component_t code_editor_component;
        sprite_renderer_component_t sprite_renderer_component;
        console_output_component_t console_output_component;
        status_panel_component_t status_panel_component;
        ecs_observer_component_t ecs_observer_component;
        project_settings_component_t project_settings_component;
        viewport_component_t viewport_component;
    };
}

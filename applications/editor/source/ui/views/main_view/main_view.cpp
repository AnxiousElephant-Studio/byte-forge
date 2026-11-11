#include <byte-forge-editor/ui/views/main_view/main_view.h>

#include <imgui.h>

namespace byteforge::editor
{
    void main_view_t::draw(editor_application_context_t& context)
    {
        const ImGuiID dockspace_id = ImGui::DockSpaceOverViewport();

        main_menu_bar_component.draw(context);
        viewport_component.draw(context, dockspace_id);
        scene_hierarchy_component.draw(context);
        object_inspector_component.draw(context);
        content_browser_component.draw(context);
        code_editor_component.draw(context);
        sprite_renderer_component.draw(context);
        console_output_component.draw(context);
        status_panel_component.draw(context);
        ecs_observer_component.draw(context);
        project_settings_component.draw(context);
    }
}

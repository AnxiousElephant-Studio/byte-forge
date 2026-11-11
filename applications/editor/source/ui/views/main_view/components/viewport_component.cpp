#include <byte-forge-editor/ui/components/viewport_component.h>
#include <byte-forge-editor/ui/utils/scoped_window.h>

#include <cstdint>

namespace byteforge::editor
{
    namespace
    {
        const char* const window_title  = "Viewport";
        const char* const input_area_id = "##viewport_input";
    }

    void viewport_component_t::draw(editor_application_context_t& context, const ImGuiID dockspace_id)
    {
        ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_FirstUseEver);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        const scoped_window_t window(window_title);

        ImGui::PopStyleVar();

        if (!window.is_visible())
        {
            return;
        }

        const ImVec2 available = ImGui::GetContentRegionAvail();
        const ImVec2 origin    = ImGui::GetCursorScreenPos();
        const ImVec2 scale     = ImGui::GetIO().DisplayFramebufferScale;

        context.engine_runtime->platform.context.input_state.set_mouse_target(math::vec2_t(origin.x, origin.y), math::vec2_t(scale.x, scale.y));

        const math::vec2i_t pixel_size(static_cast<i32>(available.x * scale.x), static_cast<i32>(available.y * scale.y));

        if (pixel_size.x > 0 && pixel_size.y > 0)
        {
            context.scene_viewport_size = pixel_size;
        }

        if (available.x <= 0.0f || available.y <= 0.0f)
        {
            return;
        }

        if (context.scene_framebuffer.is_valid())
        {
            const ImTextureID texture_id = static_cast<ImTextureID>(static_cast<std::intptr_t>(context.scene_framebuffer.get_color_texture().texture_id));

            ImGui::GetWindowDrawList()->AddImage(texture_id, origin, ImVec2(origin.x + available.x, origin.y + available.y), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
        }

        ImGui::SetNextItemAllowOverlap();
        ImGui::InvisibleButton(input_area_id, available);

        const bool is_viewport_active   = ImGui::IsItemActive();
        const bool was_viewport_clicked = ImGui::IsItemActivated();

        if (ImGui::IsItemHovered() && !context.scene_context.scene_player.is_running())
        {
            gizmo.process_shortcuts();
        }

        gizmo.update(context, origin, available, is_viewport_active, was_viewport_clicked);
        gizmo.draw_toolbar(origin);
    }
}

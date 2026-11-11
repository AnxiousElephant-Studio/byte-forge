#include <byte-forge-editor/ui/components/main_menu_bar_component.h>

#include <imgui.h>

namespace byteforge::editor
{
    namespace
    {
        const char* const project_menu_label     = "Project";
        const char* const project_settings_label = "Settings";
        const char* const save_level_label       = "Save Level";
        const char* const save_level_shortcut    = "Ctrl+S";

        const char* const view_menu_label   = "View";
        const char* const demo_window_label = "ImGui Demo";
        const char* const play_label        = "Play";
        const char* const stop_label        = "Stop";
        const char* const pause_label       = "Pause";
        const char* const resume_label      = "Resume";
        const char* const step_label        = "Step";
    }

    void main_menu_bar_component_t::draw(editor_application_context_t& context)
    {
        if (!ImGui::BeginMainMenuBar())
        {
            return;
        }

        const bool can_save_level = !context.current_level_path.empty() && !context.scene_context.scene_player.is_running();

        if (can_save_level && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_S, ImGuiInputFlags_RouteGlobal))
        {
            context.requests.is_level_save_requested = true;
        }

        if (ImGui::BeginMenu(project_menu_label))
        {
            if (ImGui::MenuItem(save_level_label, save_level_shortcut, false, can_save_level))
            {
                context.requests.is_level_save_requested = true;
            }

            ImGui::MenuItem(project_settings_label, nullptr, &context.requests.is_project_settings_window_open);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(view_menu_label))
        {
            ImGui::MenuItem(demo_window_label, nullptr, &show_demo_window);
            ImGui::EndMenu();
        }

        scene::scene_player_t& scene_player = context.scene_context.scene_player;

        if (scene_player.is_running())
        {
            if (ImGui::MenuItem(stop_label))
            {
                context.requests.is_stop_requested = true;
            }

            if (ImGui::MenuItem(scene_player.is_paused() ? resume_label : pause_label))
            {
                scene_player.set_paused(!scene_player.is_paused());
            }

            if (scene_player.is_paused() && ImGui::MenuItem(step_label))
            {
                scene_player.request_step();
            }
        }
        else if (ImGui::MenuItem(play_label))
        {
            context.requests.is_play_requested = true;
        }

        ImGui::EndMainMenuBar();

        if (show_demo_window)
        {
            ImGui::ShowDemoWindow(&show_demo_window);
        }
    }
}

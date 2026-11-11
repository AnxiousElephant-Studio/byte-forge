#include <byte-forge-editor/ui/components/project_settings_component.h>
#include <byte-forge-editor/ui/utils/editor_colors.h>

#include <byte-forge-engine/modules/project/project_file.h>

#include <algorithm>
#include <cstring>

#include <imgui.h>

namespace byteforge::editor
{
    namespace
    {
        const char* const window_title         = "Project Settings";
        const char* const general_section      = "General";
        const char* const runtime_section      = "Runtime";
        const char* const paths_section        = "Paths";
        const char* const name_label           = "Name";
        const char* const file_label           = "File";
        const char* const target_fps_label     = "Target FPS";
        const char* const vsync_label          = "VSync";
        const char* const assets_label         = "Assets folder";
        const char* const levels_label         = "Levels folder";
        const char* const vertex_shader_label  = "Sprite vertex shader";
        const char* const fragment_shader_label = "Sprite fragment shader";
        const char* const paths_hint           = "Path changes apply the next time the project is opened.";
        const char* const save_label           = "Save";
        const char* const revert_label         = "Revert";
        const char* const saved_message        = "Saved.";
        const char* const missing_name_error   = "Enter a project name.";

        const ImVec2 first_use_window_size(520.0f, 480.0f);
        const int min_target_fps = 1;
        const int max_target_fps = 240;

        template <std::size_t size>
        void copy_to_buffer(char (&buffer)[size], const std::string& text)
        {
            strncpy_s(buffer, text.c_str(), _TRUNCATE);
        }
    }

    void project_settings_component_t::draw(editor_application_context_t& context)
    {
        if (!context.requests.is_project_settings_window_open)
        {
            is_loaded = false;
            return;
        }

        if (!is_loaded)
        {
            load_from_project(context.current_project);
        }

        ImGui::SetNextWindowSize(first_use_window_size, ImGuiCond_FirstUseEver);

        if (ImGui::Begin(window_title, &context.requests.is_project_settings_window_open))
        {
            ImGui::SeparatorText(general_section);
            ImGui::InputText(name_label, name_buffer, sizeof(name_buffer));
            ImGui::TextDisabled("%s: %s", file_label, context.current_project.file_path.c_str());

            ImGui::SeparatorText(runtime_section);
            ImGui::InputInt(target_fps_label, &target_fps);
            target_fps = std::clamp(target_fps, min_target_fps, max_target_fps);
            ImGui::Checkbox(vsync_label, &use_vsync);

            ImGui::SeparatorText(paths_section);
            ImGui::InputText(assets_label, assets_directory_buffer, sizeof(assets_directory_buffer));
            ImGui::InputText(levels_label, levels_directory_buffer, sizeof(levels_directory_buffer));
            ImGui::InputText(vertex_shader_label, sprite_vertex_shader_buffer, sizeof(sprite_vertex_shader_buffer));
            ImGui::InputText(fragment_shader_label, sprite_fragment_shader_buffer, sizeof(sprite_fragment_shader_buffer));
            ImGui::TextDisabled("%s", paths_hint);

            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_Button, editor_colors_t::accent);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, editor_colors_t::accent_hover);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, editor_colors_t::accent_active);

            if (ImGui::Button(save_label))
            {
                save_to_project(context);
            }

            ImGui::PopStyleColor(3);

            ImGui::SameLine();

            if (ImGui::Button(revert_label))
            {
                load_from_project(context.current_project);
            }

            if (!status_message.empty())
            {
                ImGui::TextColored(is_status_error ? editor_colors_t::failure : editor_colors_t::success, "%s", status_message.c_str());
            }
        }

        ImGui::End();
    }

    void project_settings_component_t::load_from_project(const project::project_t& project)
    {
        const project::project_settings_t& settings = project.settings;

        copy_to_buffer(name_buffer, project.name);
        copy_to_buffer(assets_directory_buffer, settings.assets_directory);
        copy_to_buffer(levels_directory_buffer, settings.levels_directory);
        copy_to_buffer(sprite_vertex_shader_buffer, settings.sprite_vertex_shader);
        copy_to_buffer(sprite_fragment_shader_buffer, settings.sprite_fragment_shader);

        target_fps = static_cast<int>(settings.target_fps);
        use_vsync  = settings.use_vsync;

        status_message.clear();
        is_status_error = false;
        is_loaded       = true;
    }

    void project_settings_component_t::save_to_project(editor_application_context_t& context)
    {
        if (name_buffer[0] == '\0')
        {
            status_message  = missing_name_error;
            is_status_error = true;
            return;
        }

        project::project_t edited_project = context.current_project;

        edited_project.name                            = name_buffer;
        edited_project.settings.target_fps             = static_cast<u32>(target_fps);
        edited_project.settings.use_vsync              = use_vsync;
        edited_project.settings.assets_directory       = assets_directory_buffer;
        edited_project.settings.levels_directory       = levels_directory_buffer;
        edited_project.settings.sprite_vertex_shader   = sprite_vertex_shader_buffer;
        edited_project.settings.sprite_fragment_shader = sprite_fragment_shader_buffer;

        std::string error;

        if (!project::project_file_t::save(edited_project, error))
        {
            status_message  = error;
            is_status_error = true;
            return;
        }

        context.current_project = edited_project;
        context.requests.is_project_settings_apply_requested = true;

        status_message  = saved_message;
        is_status_error = false;
    }
}

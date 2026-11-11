#include <byte-forge-editor/editor_application_settings.h>
#include <byte-forge-editor/ui/utils/editor_colors.h>
#include <byte-forge-editor/ui/utils/file_name_validation.h>
#include <byte-forge-editor/ui/views/project_creation_view/project_creation_view.h>

#include <byte-forge-engine/modules/io/file_system.h>
#include <byte-forge-engine/modules/project/project_file.h>

#include <algorithm>
#include <cstring>
#include <vector>

#include <imgui.h>

namespace byteforge::editor
{
    namespace
    {
        const char* const window_id           = "##project_creation";
        const char* const templates_label     = "Templates";
        const char* const overview_label      = "Overview";
        const char* const empty_template_name = "Empty";
        const char* const empty_template_text = "A standard application project.";
        const char* const name_label          = "Project name";
        const char* const location_label      = "Location";
        const char* const browse_label        = "Browse...";
        const char* const open_label          = "Open project...";
        const char* const cancel_label        = "Cancel";
        const char* const create_label        = "Create project";

        const char* const missing_name_error     = "Enter a project name.";
        const char* const missing_location_error = "Choose a location for the project.";
        const char* const invalid_name_error     = "The project name contains characters that are not allowed in a folder name.";
        const char* const non_ascii_error        = "Project names and locations with accented characters are not supported yet.";
        const char* const shader_copy_error      = "The project was created, but the default shaders could not be copied into it.";

        const ImVec2 window_padding(28.0f, 22.0f);
        const ImVec2 template_card_size(84.0f, 92.0f);
        const float templates_column_ratio = 0.42f;
        const float bottom_section_height  = 168.0f;
        const float preview_height         = 240.0f;
        const float heading_font_size      = 22.0f;
        const float location_field_ratio   = 0.66f;
        const float action_button_height   = 36.0f;
        const float cancel_button_width    = 100.0f;
        const float create_button_width    = 120.0f;
        const float open_button_width      = 130.0f;

        void draw_grid_preview(const ImVec2 size)
        {
            const ImVec2 origin = ImGui::GetCursorScreenPos();
            const ImVec2 limit(origin.x + size.x, origin.y + size.y);
            const ImVec2 center(origin.x + size.x * 0.5f, origin.y + size.y * 0.58f);

            ImDrawList* draw_list = ImGui::GetWindowDrawList();

            draw_list->AddRectFilled(origin, limit, IM_COL32(20, 20, 20, 255), 8.0f);
            draw_list->PushClipRect(origin, limit, true);

            const float cell_width  = 24.0f;
            const float cell_height = 11.0f;
            const int half_lines    = 5;

            const auto project_point = [&](const float x, const float z)
            {
                return ImVec2(center.x + (x - z) * cell_width, center.y + (x + z) * cell_height);
            };

            const ImU32 grid_color = IM_COL32(205, 205, 205, 255);

            for (int line = -half_lines; line <= half_lines; line++)
            {
                const float position = static_cast<float>(line);
                const float extent   = static_cast<float>(half_lines);

                draw_list->AddLine(project_point(position, -extent), project_point(position, extent), grid_color);
                draw_list->AddLine(project_point(-extent, position), project_point(extent, position), grid_color);
            }

            const ImU32 cube_color = ImGui::ColorConvertFloat4ToU32(editor_colors_t::border);
            const float cube_rise  = 28.0f;

            const ImVec2 base_near  = project_point(0.5f, 0.5f);
            const ImVec2 base_left  = project_point(-0.5f, 0.5f);
            const ImVec2 base_right = project_point(0.5f, -0.5f);
            const ImVec2 base_far   = project_point(-0.5f, -0.5f);

            const ImVec2 top_near(base_near.x, base_near.y - cube_rise);
            const ImVec2 top_left(base_left.x, base_left.y - cube_rise);
            const ImVec2 top_right(base_right.x, base_right.y - cube_rise);
            const ImVec2 top_far(base_far.x, base_far.y - cube_rise);

            draw_list->AddCircleFilled(ImVec2(center.x, center.y - cube_rise * 0.5f), 34.0f, IM_COL32(91, 75, 138, 70));
            draw_list->AddQuad(top_near, top_right, top_far, top_left, cube_color, 1.5f);
            draw_list->AddLine(base_left, base_near, cube_color, 1.5f);
            draw_list->AddLine(base_near, base_right, cube_color, 1.5f);
            draw_list->AddLine(base_left, top_left, cube_color, 1.5f);
            draw_list->AddLine(base_near, top_near, cube_color, 1.5f);
            draw_list->AddLine(base_right, top_right, cube_color, 1.5f);

            draw_list->PopClipRect();

            ImGui::Dummy(size);
        }

        void copy_directory_text_files(const std::string& source_directory, const std::string& target_directory, bool& out_success)
        {
            std::vector<io::directory_entry_t> entries;

            if (!io::file_system_t::list_directory(source_directory.c_str(), entries) || !io::file_system_t::create_directories(target_directory.c_str()))
            {
                out_success = false;
                return;
            }

            for (const io::directory_entry_t& entry : entries)
            {
                if (entry.is_directory)
                {
                    continue;
                }

                std::string text;
                const std::string target_path = io::file_system_t::join(target_directory.c_str(), entry.name.c_str());

                if (!io::file_system_t::read_text(entry.path.c_str(), text) || !io::file_system_t::write_text(target_path.c_str(), text))
                {
                    out_success = false;
                }
            }
        }
    }

    void project_creation_view_t::draw(editor_application_context_t& context)
    {
        if (!context.requests.project_open_error.empty())
        {
            error_message = context.requests.project_open_error;
            context.requests.project_open_error.clear();
        }

        std::string selected_path;

        if (location_dialog.take_selected_path(selected_path))
        {
            strncpy_s(location_buffer, selected_path.c_str(), _TRUNCATE);
        }

        if (open_project_dialog.take_selected_path(selected_path))
        {
            context.requests.project_file_to_open = selected_path;
        }

        const ImGuiViewport* viewport = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        const ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, window_padding);

        if (ImGui::Begin(window_id, nullptr, window_flags))
        {
            const ImVec2 available = ImGui::GetContentRegionAvail();
            const float top_height = std::max(available.y - bottom_section_height, preview_height);

            content_width = available.x;

            const float templates_width = content_width * templates_column_ratio;

            ImGui::BeginChild("##templates", ImVec2(templates_width, top_height), ImGuiChildFlags_None);
            draw_templates();
            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::BeginChild("##overview_column", ImVec2(0.0f, top_height), ImGuiChildFlags_None);
            draw_overview();
            ImGui::EndChild();

            ImGui::Separator();

            draw_project_fields(context);
            draw_actions(context);
        }

        ImGui::End();
        ImGui::PopStyleVar(3);
    }

    void project_creation_view_t::draw_templates()
    {
        ImGui::TextUnformatted(templates_label);

        const ImVec2 card_start = ImGui::GetCursorScreenPos();
        const ImVec2 card_end(card_start.x + template_card_size.x, card_start.y + template_card_size.y);
        const ImVec2 text_size = ImGui::CalcTextSize(empty_template_name);

        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        draw_list->AddRectFilled(card_start, card_end, ImGui::ColorConvertFloat4ToU32(editor_colors_t::accent_subtle), 8.0f);
        draw_list->AddRect(card_start, card_end, ImGui::ColorConvertFloat4ToU32(editor_colors_t::accent), 8.0f);
        draw_list->AddText(ImVec2(card_start.x + (template_card_size.x - text_size.x) * 0.5f, card_start.y + (template_card_size.y - text_size.y) * 0.5f), ImGui::GetColorU32(ImGuiCol_Text), empty_template_name);

        ImGui::Dummy(template_card_size);
    }

    void project_creation_view_t::draw_overview()
    {
        ImGui::TextUnformatted(overview_label);

        ImGui::PushStyleColor(ImGuiCol_ChildBg, editor_colors_t::panel);
        ImGui::BeginChild("##overview", ImVec2(0.0f, 0.0f), ImGuiChildFlags_Borders);
        ImGui::PopStyleColor();

        draw_grid_preview(ImVec2(ImGui::GetContentRegionAvail().x, preview_height));

        ImGui::Spacing();
        ImGui::PushFont(nullptr, heading_font_size);
        ImGui::TextUnformatted(empty_template_name);
        ImGui::PopFont();
        ImGui::TextDisabled("%s", empty_template_text);

        ImGui::EndChild();
    }

    void project_creation_view_t::draw_project_fields(editor_application_context_t& context)
    {
        const float spacing        = ImGui::GetStyle().ItemSpacing.x;
        const float name_width     = content_width * (1.0f - location_field_ratio) - spacing;
        const float browse_width   = ImGui::CalcTextSize(browse_label).x + ImGui::GetStyle().FramePadding.x * 2.0f;
        const float location_width = content_width * location_field_ratio - browse_width - spacing;
        const float location_start = ImGui::GetCursorPosX() + name_width + spacing;

        ImGui::TextDisabled("%s", name_label);
        ImGui::SameLine();
        ImGui::SetCursorPosX(location_start);
        ImGui::TextDisabled("%s", location_label);

        ImGui::SetNextItemWidth(name_width);
        ImGui::InputText("##project_name", name_buffer, sizeof(name_buffer));
        ImGui::SameLine();
        ImGui::SetNextItemWidth(location_width);
        ImGui::InputText("##project_location", location_buffer, sizeof(location_buffer));
        ImGui::SameLine();

        if (ImGui::Button(browse_label))
        {
            location_dialog.request_folder(context.engine_runtime->platform.get_current_window()->sdl_window);
        }
    }

    void project_creation_view_t::draw_actions(editor_application_context_t& context)
    {
        ImGui::Spacing();

        if (!error_message.empty())
        {
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + content_width);
            ImGui::TextColored(editor_colors_t::failure, "%s", error_message.c_str());
            ImGui::PopTextWrapPos();
        }

        const float group_start = ImGui::GetCursorPosX();
        const float spacing     = ImGui::GetStyle().ItemSpacing.x;

        if (ImGui::Button(open_label, ImVec2(open_button_width, action_button_height)))
        {
            open_project_dialog.request_project_file(context.engine_runtime->platform.get_current_window()->sdl_window);
        }

        ImGui::SameLine();
        ImGui::SetCursorPosX(group_start + content_width - create_button_width - cancel_button_width - spacing);

        if (ImGui::Button(cancel_label, ImVec2(cancel_button_width, action_button_height)))
        {
            context.engine_runtime->platform.request_close();
        }

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, editor_colors_t::accent);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, editor_colors_t::accent_hover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, editor_colors_t::accent_active);

        if (ImGui::Button(create_label, ImVec2(create_button_width, action_button_height)))
        {
            create_project(context);
        }

        ImGui::PopStyleColor(3);
    }

    void project_creation_view_t::create_project(editor_application_context_t& context)
    {
        error_message.clear();

        if (name_buffer[0] == '\0')
        {
            error_message = missing_name_error;
            return;
        }

        if (location_buffer[0] == '\0')
        {
            error_message = missing_location_error;
            return;
        }

        if (has_invalid_file_name_characters(name_buffer))
        {
            error_message = invalid_name_error;
            return;
        }

        if (!is_ascii_text(name_buffer) || !is_ascii_text(location_buffer))
        {
            error_message = non_ascii_error;
            return;
        }

        project::project_t project;

        if (!project::project_file_t::create(name_buffer, location_buffer, project, error_message))
        {
            return;
        }

        bool copied_shaders = true;

        copy_directory_text_files(
            io::file_system_t::join(context.editor_directory.c_str(), editor_application_settings_t::template_shaders_directory),
            io::file_system_t::join(project.root_directory.c_str(), editor_application_settings_t::project_shaders_directory),
            copied_shaders);

        if (!copied_shaders)
        {
            error_message = shader_copy_error;
            return;
        }

        context.requests.project_file_to_open = project.file_path;
    }
}

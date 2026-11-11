#include <byte-forge-editor/ui/components/console_output_component.h>
#include <byte-forge-editor/ui/utils/scoped_window.h>

#include <byte-forge-engine/modules/logging/e_log_levels.h>
#include <byte-forge-engine/modules/logging/log_buffer.h>
#include <byte-forge-engine/modules/logging/logger.h>

#include <imgui.h>

#include <string>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <iterator>
#include <vector>

namespace byteforge::editor
{
    namespace
    {
        const char* const window_title          = "Console Output";
        const char* const displayed_level_label = "Display level";
        const char* const captured_level_label  = "Capture level";
        const char* const search_query_label    = "Search";
        const char* const auto_scroll_label     = "Auto-scroll";
        const char* const clear_label           = "Clear";
        const char* const summary_format        = "%zu shown / %zu kept / %llu total  |  %zu warnings, %zu errors";

        const usize summary_buffer_size = 160;

        const f32 log_level_combo_width = 100.0f;
        const f32 search_query_width    = 180.0f;

        const char* const lines_child_id = "console_lines";

        const char* const level_names[] = { "Info", "Debug", "Trace", "Success", "Warning", "Error", "Fatal" };

        const ImVec4 level_colors[] =
        {
            ImVec4(0.90f, 0.90f, 0.90f, 1.0f),
            ImVec4(0.40f, 0.85f, 0.90f, 1.0f),
            ImVec4(0.45f, 0.55f, 0.95f, 1.0f),
            ImVec4(0.45f, 0.85f, 0.45f, 1.0f),
            ImVec4(0.95f, 0.80f, 0.30f, 1.0f),
            ImVec4(0.95f, 0.40f, 0.40f, 1.0f),
            ImVec4(1.00f, 0.25f, 0.55f, 1.0f),
        };

        bool contains_ignoring_case(const std::string& text, const char* search_query)
        {
            const std::string needle(search_query);

            return std::search(text.begin(), text.end(), needle.begin(), needle.end(), [](char left, char right)
            {
                return std::tolower(static_cast<unsigned char>(left)) == std::tolower(static_cast<unsigned char>(right));
            }) != text.end();
        }

        logging::e_log_levels draw_log_level_combo(const char* label, const logging::e_log_levels current_log_level)
        {
            i32 selected_log_level = static_cast<i32>(current_log_level);

            ImGui::SetNextItemWidth(log_level_combo_width);
            ImGui::Combo(label, &selected_log_level, level_names, static_cast<i32>(std::size(level_names)));

            return static_cast<logging::e_log_levels>(selected_log_level);
        }
    }

    void console_output_component_t::draw(editor_application_context_t&)
    {
        const scoped_window_t window(window_title);

        if (!window.is_visible())
        {
            return;
        }

        logging::log_buffer_t& buffer = logging::logger_t::get_buffer();

        const logging::e_log_levels minimum_captured_log_level  = logging::logger_t::get_min_level();
        const logging::e_log_levels selected_captured_log_level = draw_log_level_combo(captured_level_label, minimum_captured_log_level);

        if (selected_captured_log_level != minimum_captured_log_level)
        {
            logging::logger_t::set_min_level(selected_captured_log_level);
        }

        ImGui::SameLine();
        minimum_displayed_log_level = draw_log_level_combo(displayed_level_label, minimum_displayed_log_level);

        ImGui::SameLine();
        ImGui::SetNextItemWidth(search_query_width);
        ImGui::InputText(search_query_label, search_query, sizeof(search_query));

        ImGui::SameLine();
        ImGui::Checkbox(auto_scroll_label, &is_auto_scroll_enabled);

        ImGui::SameLine();

        if (ImGui::Button(clear_label))
        {
            buffer.clear();
        }

        std::vector<usize> visible_entries;
        usize warning_count = 0;
        usize error_count   = 0;

        for (usize index = 0; index < buffer.get_count(); index++)
        {
            const logging::log_entry_t& entry = buffer.get(index);

            if (entry.level == logging::e_log_levels::Warning)
            {
                warning_count++;
            }
            else if (entry.level >= logging::e_log_levels::Error)
            {
                error_count++;
            }

            if (entry.level >= minimum_displayed_log_level && (search_query[0] == '\0' || contains_ignoring_case(entry.line, search_query)))
            {
                visible_entries.push_back(index);
            }
        }

        char summary[summary_buffer_size];
        std::snprintf(summary, sizeof(summary), summary_format, visible_entries.size(), buffer.get_count(), static_cast<unsigned long long>(buffer.get_total_count()), warning_count, error_count);
        ImGui::TextUnformatted(summary);

        ImGui::Separator();
        ImGui::BeginChild(lines_child_id, ImVec2(), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);

        ImGuiListClipper clipper;
        clipper.Begin(static_cast<i32>(visible_entries.size()));

        while (clipper.Step())
        {
            for (i32 row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
            {
                const logging::log_entry_t& entry = buffer.get(visible_entries[static_cast<usize>(row)]);

                ImGui::PushStyleColor(ImGuiCol_Text, level_colors[static_cast<usize>(entry.level)]);
                ImGui::TextUnformatted(entry.line.c_str());
                ImGui::PopStyleColor();
            }
        }

        if (is_auto_scroll_enabled && buffer.get_total_count() != last_seen_total_log_count)
        {
            ImGui::SetScrollHereY(1.0f);
        }

        last_seen_total_log_count = buffer.get_total_count();

        ImGui::EndChild();
    }
}

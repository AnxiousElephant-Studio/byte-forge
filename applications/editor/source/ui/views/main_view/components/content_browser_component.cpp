#include <byte-forge-editor/ui/components/content_browser_component.h>
#include <byte-forge-editor/ui/utils/drag_drop.h>
#include <byte-forge-editor/ui/utils/editor_colors.h>
#include <byte-forge-editor/ui/utils/file_name_validation.h>
#include <byte-forge-editor/ui/utils/scoped_window.h>

#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/level/stamp_file.h>
#include <byte-forge-engine/modules/rendering/sprite/sprite_material_template.h>

#include <imgui.h>

#include <cstdio>

namespace byteforge::editor
{
    namespace
    {
        const char* const window_title             = "Content Browser";
        const char* const back_label               = "<";
        const char* const refresh_label            = "Refresh";
        const char* const new_folder_label         = "New Folder";
        const char* const new_script_label         = "New Script";
        const char* const new_material_label       = "New Material";
        const char* const rename_label             = "Rename";
        const char* const delete_label             = "Delete";
        const char* const delete_popup_id          = "Delete Asset";
        const char* const delete_message_format    = "Delete '%s'? This cannot be undone.";
        const char* const cancel_label             = "Cancel";
        const char* const delete_failed_error      = "Could not delete. The file may be open in another program.";
        const char* const rename_shortcut_label    = "F2";
        const char* const browser_context_popup_id = "content_browser_context";
        const char* const rename_field_id          = "##rename";
        const char* const directory_format         = "[%s]";
        const char* const file_format              = "%s";
        const char* const script_extension         = ".lua";
        const char* const shader_extension         = ".glsl";
        const char* const new_folder_format        = "folder_%u";
        const char* const new_script_format        = "script_%u.lua";
        const char* const new_script_template      = "function on_start(entity)\nend\n\nfunction on_update(entity, delta_time)\nend\n";
        const char* const new_material_format      = "material_%u.glsl";
        const char* const new_material_template    = rendering::sprite_material_template;

        const char* const empty_name_error    = "Enter a name.";
        const char* const invalid_name_error  = "The name contains characters that are not allowed in a file name.";
        const char* const non_ascii_error     = "Names with accented characters are not supported yet.";
        const char* const existing_name_error = "A file or folder with this name already exists.";
        const char* const rename_failed_error = "Could not rename. The file may be open in another program.";

        const usize file_name_buffer_size = 64;
        const u32 max_new_file_attempts   = 1000;
    }

    void content_browser_component_t::draw(editor_application_context_t& context)
    {
        if (current_directory.empty())
        {
            current_directory = context.assets_directory;
        }

        if (needs_refresh)
        {
            refresh();
        }

        const scoped_window_t window(window_title);

        if (!window.is_visible())
        {
            return;
        }

        draw_toolbar(context);

        if (!file_error.empty())
        {
            ImGui::TextColored(editor_colors_t::failure, "%s", file_error.c_str());
        }

        ImGui::Separator();

        for (usize index = 0; index < entries.size(); index++)
        {
            ImGui::PushID(static_cast<i32>(index));
            draw_entry(context, entries[index]);
            ImGui::PopID();
        }

        if (ImGui::BeginPopupContextWindow(browser_context_popup_id, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
        {
            draw_create_items();
            ImGui::EndPopup();
        }

        draw_delete_popup();
    }

    void content_browser_component_t::draw_delete_popup()
    {
        if (should_open_delete_popup)
        {
            should_open_delete_popup = false;
            ImGui::OpenPopup(delete_popup_id);
        }

        if (!ImGui::BeginPopupModal(delete_popup_id, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            return;
        }

        ImGui::Text(delete_message_format, io::file_system_t::get_file_name(deleting_path.c_str()).c_str());
        ImGui::Spacing();

        if (ImGui::Button(delete_label))
        {
            file_error.clear();

            if (!io::file_system_t::remove(deleting_path.c_str()))
            {
                file_error = delete_failed_error;
            }

            deleting_path.clear();
            needs_refresh = true;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button(cancel_label))
        {
            deleting_path.clear();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    void content_browser_component_t::draw_toolbar(editor_application_context_t& context)
    {
        ImGui::BeginDisabled(current_directory == context.assets_directory);

        if (ImGui::Button(back_label))
        {
            current_directory = io::file_system_t::get_parent_path(current_directory.c_str());
            cancel_rename();
            needs_refresh = true;
        }

        ImGui::EndDisabled();
        ImGui::SameLine();

        if (ImGui::Button(refresh_label))
        {
            needs_refresh = true;
        }

        ImGui::SameLine();

        if (ImGui::Button(new_folder_label))
        {
            begin_rename(create_folder());
        }

        ImGui::SameLine();

        if (ImGui::Button(new_script_label))
        {
            begin_rename(create_file(new_script_format, new_script_template));
        }

        ImGui::SameLine();

        if (ImGui::Button(new_material_label))
        {
            begin_rename(create_file(new_material_format, new_material_template));
        }

        ImGui::SameLine();
        ImGui::TextUnformatted(current_directory.c_str());
    }

    void content_browser_component_t::draw_create_items()
    {
        if (ImGui::MenuItem(new_folder_label))
        {
            begin_rename(create_folder());
        }

        if (ImGui::MenuItem(new_script_label))
        {
            begin_rename(create_file(new_script_format, new_script_template));
        }

        if (ImGui::MenuItem(new_material_label))
        {
            begin_rename(create_file(new_material_format, new_material_template));
        }
    }

    void content_browser_component_t::draw_entry(editor_application_context_t& context, const io::directory_entry_t& entry)
    {
        if (entry.path == renaming_path)
        {
            draw_rename_field();
            return;
        }

        char label[file_name_buffer_size * 2];
        std::snprintf(label, sizeof(label), entry.is_directory ? directory_format : file_format, entry.name.c_str());

        const bool is_code_file = !entry.is_directory && (entry.extension == script_extension || entry.extension == shader_extension);

        if (ImGui::Selectable(label, false, ImGuiSelectableFlags_AllowDoubleClick) && (is_code_file || ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)))
        {
            open_entry(context, entry);
        }

        if (ImGui::IsItemHovered() && ImGui::IsKeyPressed(ImGuiKey_F2))
        {
            begin_rename(entry.path);
        }

        if (!entry.is_directory)
        {
            begin_asset_path_drag(entry.path, entry.name.c_str());
        }

        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem(rename_label, rename_shortcut_label))
            {
                begin_rename(entry.path);
            }

            if (ImGui::MenuItem(delete_label))
            {
                deleting_path            = entry.path;
                should_open_delete_popup = true;
            }

            ImGui::Separator();
            draw_create_items();
            ImGui::EndPopup();
        }
    }

    void content_browser_component_t::draw_rename_field()
    {
        if (should_focus_rename)
        {
            ImGui::SetKeyboardFocusHere();
            should_focus_rename = false;
        }

        ImGui::SetNextItemWidth(-1.0f);

        const bool submitted = ImGui::InputText(rename_field_id, rename_buffer, sizeof(rename_buffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll);

        if (ImGui::IsItemActive() && ImGui::IsKeyPressed(ImGuiKey_Escape))
        {
            cancel_rename();
            return;
        }

        if (submitted || ImGui::IsItemDeactivated())
        {
            apply_rename();
        }
    }

    void content_browser_component_t::open_entry(editor_application_context_t& context, const io::directory_entry_t& entry)
    {
        if (entry.is_directory)
        {
            current_directory = entry.path;
            cancel_rename();
            needs_refresh = true;
        }
        else if (entry.extension == script_extension || entry.extension == shader_extension)
        {
            context.requests.file_to_edit = entry.path;
        }
        else if (entry.extension == level::stamp_file_t::extension)
        {
            context.requests.stamp_to_instantiate = entry.path;
        }
    }

    void content_browser_component_t::refresh()
    {
        io::file_system_t::list_directory(current_directory.c_str(), entries);
        needs_refresh = false;
    }

    std::string content_browser_component_t::create_file(const char* file_name_format, const char* content)
    {
        char file_name[file_name_buffer_size];

        for (u32 attempt = 0; attempt < max_new_file_attempts; attempt++)
        {
            std::snprintf(file_name, sizeof(file_name), file_name_format, attempt);
            const std::string path = io::file_system_t::join(current_directory.c_str(), file_name);

            if (!io::file_system_t::exists(path.c_str()))
            {
                needs_refresh = true;
                return io::file_system_t::write_text(path.c_str(), content) ? path : std::string();
            }
        }

        return std::string();
    }

    std::string content_browser_component_t::create_folder()
    {
        char folder_name[file_name_buffer_size];

        for (u32 attempt = 0; attempt < max_new_file_attempts; attempt++)
        {
            std::snprintf(folder_name, sizeof(folder_name), new_folder_format, attempt);
            const std::string path = io::file_system_t::join(current_directory.c_str(), folder_name);

            if (!io::file_system_t::exists(path.c_str()))
            {
                needs_refresh = true;
                return io::file_system_t::create_directories(path.c_str()) ? path : std::string();
            }
        }

        return std::string();
    }

    void content_browser_component_t::begin_rename(const std::string& path)
    {
        if (path.empty())
        {
            return;
        }

        renaming_path       = path;
        should_focus_rename = true;
        file_error.clear();

        std::snprintf(rename_buffer, sizeof(rename_buffer), file_format, io::file_system_t::get_file_name(path.c_str()).c_str());
    }

    void content_browser_component_t::apply_rename()
    {
        const std::string path     = renaming_path;
        const std::string new_name = rename_buffer;

        cancel_rename();

        if (new_name == io::file_system_t::get_file_name(path.c_str()))
        {
            return;
        }

        if (new_name.empty())
        {
            file_error = empty_name_error;
        }
        else if (has_invalid_file_name_characters(new_name.c_str()))
        {
            file_error = invalid_name_error;
        }
        else if (!is_ascii_text(new_name.c_str()))
        {
            file_error = non_ascii_error;
        }

        if (!file_error.empty())
        {
            return;
        }

        const std::string new_path = io::file_system_t::join(io::file_system_t::get_parent_path(path.c_str()).c_str(), new_name.c_str());

        if (io::file_system_t::exists(new_path.c_str()))
        {
            file_error = existing_name_error;
            return;
        }

        if (!io::file_system_t::rename(path.c_str(), new_path.c_str()))
        {
            file_error = rename_failed_error;
            return;
        }

        needs_refresh = true;
    }

    void content_browser_component_t::cancel_rename()
    {
        renaming_path.clear();
        should_focus_rename = false;
    }
}

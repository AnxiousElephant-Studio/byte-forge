#include <byte-forge-editor/ui/components/code_editor_component.h>
#include <byte-forge-editor/ui/utils/scoped_window.h>

#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/io/file_system.h>
#include <byte-forge-engine/modules/logging/log_macros.h>

#include <imgui.h>

namespace byteforge::editor
{
    namespace
    {
        const char* const window_title      = "Code Editor";
        const char* const save_label        = "Save";
        const char* const text_label        = "##code_text";
        const char* const dirty_marker      = "*";
        const char* const no_file_label     = "Double-click a code file in the Content Browser or use Edit Script in the Object Inspector.";
        const char* const open_error_format = "Could not open file at path: '%s'.";
        const char* const save_error_format = "Could not save file at path: '%s'.";
        const char* const shader_extension  = ".glsl";

        i32 resize_text_callback(ImGuiInputTextCallbackData* data)
        {
            if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
            {
                std::string* text = static_cast<std::string*>(data->UserData);
                text->resize(static_cast<usize>(data->BufTextLen));
                data->Buf = text->data();
            }

            return 0;
        }
    }

    void code_editor_component_t::open(const std::string& path)
    {
        std::string loaded_text;

        if (!io::file_system_t::read_text(path.c_str(), loaded_text))
        {
            LOG_ERROR(open_error_format, path.c_str());
            return;
        }

        file_path = path;
        file_content = loaded_text;
        is_open = true;
        is_dirty = false;
    }

    bool code_editor_component_t::save()
    {
        if (!io::file_system_t::write_text(file_path.c_str(), file_content))
        {
            LOG_ERROR(save_error_format, file_path.c_str());
            return false;
        }

        is_dirty = false;

        return true;
    }

    void code_editor_component_t::draw(editor_application_context_t& context)
    {
        if (!context.requests.file_to_edit.empty())
        {
            open(context.requests.file_to_edit);
            context.requests.file_to_edit.clear();
            ImGui::SetNextWindowFocus();
        }

        const scoped_window_t window(window_title);

        if (!window.is_visible())
        {
            return;
        }

        if (!is_open)
        {
            ImGui::PushTextWrapPos();
            ImGui::TextUnformatted(no_file_label);
            ImGui::PopTextWrapPos();
            return;
        }

        if (ImGui::Button(save_label) || (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_S)))
        {
            if (save() && io::file_system_t::get_extension(file_path.c_str()) == shader_extension)
            {
                context.requests.is_sprite_shader_reload_requested = true;
            }
        }

        ImGui::SameLine();
        ImGui::TextUnformatted(file_path.c_str());

        if (is_dirty)
        {
            ImGui::SameLine();
            ImGui::TextUnformatted(dirty_marker);
        }

        const ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_CallbackResize;

        if (ImGui::InputTextMultiline(text_label, file_content.data(), file_content.capacity() + 1, ImVec2(-1.0f, -1.0f), flags, resize_text_callback, &file_content))
        {
            is_dirty = true;
        }
    }
}

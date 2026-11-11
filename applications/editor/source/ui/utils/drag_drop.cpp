#include <byte-forge-editor/ui/utils/drag_drop.h>

#include <imgui.h>

#include <cstdio>

namespace byteforge::editor
{
    namespace
    {
        const char* const path_format = "%s";

        const char* accept_asset_path_payload()
        {
            if (!ImGui::BeginDragDropTarget())
            {
                return nullptr;
            }

            const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(drag_drop_payloads_t::asset_path);
            ImGui::EndDragDropTarget();

            return payload != nullptr ? static_cast<const char*>(payload->Data) : nullptr;
        }
    }

    void begin_asset_path_drag(const std::string& path, const char* preview_label)
    {
        if (!ImGui::BeginDragDropSource())
        {
            return;
        }

        ImGui::SetDragDropPayload(drag_drop_payloads_t::asset_path, path.c_str(), path.size() + 1);
        ImGui::TextUnformatted(preview_label);
        ImGui::EndDragDropSource();
    }

    bool accept_asset_path_drop(std::string& out_path)
    {
        const char* dropped_path = accept_asset_path_payload();

        if (dropped_path == nullptr)
        {
            return false;
        }

        out_path = dropped_path;

        return true;
    }

    bool accept_asset_path_drop(char* out_buffer, const usize buffer_size)
    {
        const char* dropped_path = accept_asset_path_payload();

        if (dropped_path == nullptr)
        {
            return false;
        }

        std::snprintf(out_buffer, buffer_size, path_format, dropped_path);

        return true;
    }
}

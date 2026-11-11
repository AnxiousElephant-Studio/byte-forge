#pragma once

#include <byte-forge-editor/editor_application_context.h>
#include <byte-forge-editor/ui/utils/viewport_gizmo.h>

#include <imgui.h>

namespace byteforge::editor
{
    class viewport_component_t
    {
    public:
        void draw(editor_application_context_t& context, const ImGuiID dockspace_id);

    private:
        viewport_gizmo_t gizmo;
    };
}

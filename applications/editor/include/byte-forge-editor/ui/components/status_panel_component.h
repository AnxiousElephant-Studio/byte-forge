#pragma once

#include <byte-forge-editor/editor_application_context.h>

#include <byte-forge-engine/api/types.h>

#include <vector>

namespace byteforge::editor
{
    class status_panel_component_t
    {
    public:
        status_panel_component_t() : frame_history(), history_offset(0) {}

        void draw(editor_application_context_t& context);

    private:
        void draw_frame(editor_application_context_t& context);
        void draw_phases(editor_application_context_t& context);
        void draw_scene(editor_application_context_t& context);
        void draw_resources(editor_application_context_t& context);
        void draw_controls(editor_application_context_t& context);

        std::vector<f32> frame_history;
        usize history_offset;
    };
}

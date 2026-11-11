#pragma once

#include <byte-forge-editor/editor_application_context.h>
#include <byte-forge-editor/ui/utils/native_file_dialog.h>

#include <string>

namespace byteforge::editor
{
    class project_creation_view_t
    {
    public:
        void draw(editor_application_context_t& context);

    private:
        void draw_templates();
        void draw_overview();
        void draw_project_fields(editor_application_context_t& context);
        void draw_actions(editor_application_context_t& context);

        void create_project(editor_application_context_t& context);

        native_file_dialog_t location_dialog;
        native_file_dialog_t open_project_dialog;

        char name_buffer[128]      = {};
        char location_buffer[1024] = {};
        std::string error_message;
        float content_width = 0.0f;
    };
}

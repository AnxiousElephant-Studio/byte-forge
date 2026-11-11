#pragma once

#include <byte-forge-editor/editor_application_context.h>

namespace byteforge::editor
{
    class main_menu_bar_component_t
    {
    public:
        main_menu_bar_component_t() : show_demo_window(false) {}

        void draw(editor_application_context_t& context);

    private:
        bool show_demo_window;
    };
}

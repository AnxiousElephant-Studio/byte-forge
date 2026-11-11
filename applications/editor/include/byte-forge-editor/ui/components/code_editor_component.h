#pragma once

#include <byte-forge-editor/editor_application_context.h>

#include <string>

namespace byteforge::editor
{
    class code_editor_component_t
    {
    public:
        code_editor_component_t() : file_path(), file_content(), is_open(false), is_dirty(false) {}

        void draw(editor_application_context_t& context);

    private:
        void open(const std::string& path);
        bool save();

        std::string file_path;
        std::string file_content;
        bool is_open;
        bool is_dirty;
    };
}

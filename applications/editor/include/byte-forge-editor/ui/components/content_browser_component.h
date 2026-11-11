#pragma once

#include <byte-forge-editor/editor_application_context.h>

#include <byte-forge-engine/modules/io/file_system.h>

#include <string>
#include <vector>

namespace byteforge::editor
{
    class content_browser_component_t
    {
    public:
        content_browser_component_t() : current_directory(), entries(), needs_refresh(true), renaming_path(), rename_buffer(), file_error(), should_focus_rename(false) {}

        void draw(editor_application_context_t& context);

    private:
        void refresh();
        void draw_toolbar(editor_application_context_t& context);
        void draw_create_items();
        void draw_entry(editor_application_context_t& context, const io::directory_entry_t& entry);
        void draw_rename_field();
        void open_entry(editor_application_context_t& context, const io::directory_entry_t& entry);

        std::string create_file(const char* file_name_format, const char* content);
        std::string create_folder();
        void begin_rename(const std::string& path);
        void apply_rename();
        void cancel_rename();
        void draw_delete_popup();

        std::string current_directory;
        std::vector<io::directory_entry_t> entries;
        bool needs_refresh;

        std::string renaming_path;
        char rename_buffer[128];
        std::string file_error;
        bool should_focus_rename;

        std::string deleting_path;
        bool should_open_delete_popup = false;
    };
}

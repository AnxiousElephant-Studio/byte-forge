#pragma once

#include <byte-forge-engine/modules/platform/display/window/window.h>

#include <string>

namespace byteforge::editor
{
    class debug_ui_context_t
    {
    public:
        debug_ui_context_t() : initialized(false) {}

        bool initialize(const platform::window_t& window);
        void dispose();

        void process_event(const void* native_event) const;

        void begin_frame() const;
        void end_frame() const;

    private:
        bool initialized;
        std::string layout_file_path;
    };
}

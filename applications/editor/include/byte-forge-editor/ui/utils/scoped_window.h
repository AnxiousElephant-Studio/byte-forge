#pragma once

#include <imgui.h>

namespace byteforge::editor
{
    class scoped_window_t
    {
    public:
        explicit scoped_window_t(const char* title) : visible(ImGui::Begin(title)) {}

        ~scoped_window_t()
        {
            ImGui::End();
        }

        scoped_window_t(const scoped_window_t&)            = delete;
        scoped_window_t& operator=(const scoped_window_t&) = delete;

        bool is_visible() const
        {
            return visible;
        }

    private:
        bool visible;
    };
}

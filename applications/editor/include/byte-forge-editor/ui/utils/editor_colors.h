#pragma once

#include <imgui.h>

namespace byteforge::editor
{
    struct editor_colors_t
    {
    public:
        static constexpr ImVec4 success = ImVec4(0.45f, 0.85f, 0.45f, 1.0f);
        static constexpr ImVec4 failure = ImVec4(0.95f, 0.40f, 0.40f, 1.0f);

        static constexpr ImVec4 background     = ImVec4(0.118f, 0.118f, 0.118f, 1.0f);
        static constexpr ImVec4 panel          = ImVec4(0.176f, 0.176f, 0.176f, 1.0f);
        static constexpr ImVec4 control        = ImVec4(0.200f, 0.200f, 0.200f, 1.0f);
        static constexpr ImVec4 control_hover  = ImVec4(0.251f, 0.251f, 0.251f, 1.0f);
        static constexpr ImVec4 control_active = ImVec4(0.290f, 0.290f, 0.290f, 1.0f);

        static constexpr ImVec4 accent        = ImVec4(0.290f, 0.549f, 1.000f, 1.0f);
        static constexpr ImVec4 accent_hover  = ImVec4(0.420f, 0.631f, 1.000f, 1.0f);
        static constexpr ImVec4 accent_active = ImVec4(0.227f, 0.471f, 0.910f, 1.0f);
        static constexpr ImVec4 accent_subtle = ImVec4(0.290f, 0.549f, 1.000f, 0.25f);

        static constexpr ImVec4 border = ImVec4(0.357f, 0.294f, 0.541f, 1.0f);

        static constexpr ImVec4 text          = ImVec4(0.949f, 0.949f, 0.949f, 1.0f);
        static constexpr ImVec4 text_disabled = ImVec4(0.549f, 0.549f, 0.549f, 1.0f);
    };
}

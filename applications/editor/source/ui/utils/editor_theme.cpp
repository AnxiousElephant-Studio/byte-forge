#include <byte-forge-editor/ui/utils/editor_colors.h>
#include <byte-forge-editor/ui/utils/editor_theme.h>

#include <filesystem>

#include <imgui.h>

namespace byteforge::editor
{
    namespace
    {
        const char* const ui_font_path = "C:/Windows/Fonts/segoeui.ttf";
        const float ui_font_size       = 16.0f;

        void apply_font()
        {
            std::error_code error;

            if (std::filesystem::exists(ui_font_path, error))
            {
                ImGui::GetIO().Fonts->AddFontFromFileTTF(ui_font_path, ui_font_size);
            }
        }

        void apply_metrics(ImGuiStyle& style)
        {
            style.WindowPadding     = ImVec2(12.0f, 12.0f);
            style.FramePadding      = ImVec2(10.0f, 6.0f);
            style.ItemSpacing       = ImVec2(8.0f, 8.0f);
            style.ItemInnerSpacing  = ImVec2(6.0f, 6.0f);
            style.ScrollbarSize     = 12.0f;
            style.GrabMinSize       = 10.0f;

            style.WindowRounding    = 8.0f;
            style.ChildRounding     = 8.0f;
            style.PopupRounding     = 6.0f;
            style.FrameRounding     = 6.0f;
            style.ScrollbarRounding = 6.0f;
            style.GrabRounding      = 4.0f;
            style.TabRounding       = 6.0f;

            style.WindowBorderSize  = 1.0f;
            style.PopupBorderSize   = 1.0f;
            style.FrameBorderSize   = 0.0f;
        }

        void apply_colors(ImGuiStyle& style)
        {
            using colors_t = editor_colors_t;

            ImVec4* colors = style.Colors;

            colors[ImGuiCol_Text]                 = colors_t::text;
            colors[ImGuiCol_TextDisabled]         = colors_t::text_disabled;
            colors[ImGuiCol_TextSelectedBg]       = colors_t::accent_subtle;

            colors[ImGuiCol_WindowBg]             = colors_t::background;
            colors[ImGuiCol_ChildBg]              = colors_t::background;
            colors[ImGuiCol_PopupBg]              = colors_t::panel;
            colors[ImGuiCol_MenuBarBg]            = colors_t::background;
            colors[ImGuiCol_DockingEmptyBg]       = colors_t::background;

            colors[ImGuiCol_Border]               = colors_t::border;
            colors[ImGuiCol_Separator]            = colors_t::border;
            colors[ImGuiCol_SeparatorHovered]     = colors_t::accent_hover;
            colors[ImGuiCol_SeparatorActive]      = colors_t::accent;

            colors[ImGuiCol_FrameBg]              = colors_t::control;
            colors[ImGuiCol_FrameBgHovered]       = colors_t::control_hover;
            colors[ImGuiCol_FrameBgActive]        = colors_t::control_active;

            colors[ImGuiCol_TitleBg]              = colors_t::background;
            colors[ImGuiCol_TitleBgActive]        = colors_t::panel;
            colors[ImGuiCol_TitleBgCollapsed]     = colors_t::background;

            colors[ImGuiCol_ScrollbarBg]          = colors_t::background;
            colors[ImGuiCol_ScrollbarGrab]        = colors_t::control_hover;
            colors[ImGuiCol_ScrollbarGrabHovered] = colors_t::control_active;
            colors[ImGuiCol_ScrollbarGrabActive]  = colors_t::accent;

            colors[ImGuiCol_CheckMark]            = colors_t::accent;
            colors[ImGuiCol_SliderGrab]           = colors_t::accent;
            colors[ImGuiCol_SliderGrabActive]     = colors_t::accent_active;

            colors[ImGuiCol_Button]               = colors_t::control;
            colors[ImGuiCol_ButtonHovered]        = colors_t::control_hover;
            colors[ImGuiCol_ButtonActive]         = colors_t::control_active;

            colors[ImGuiCol_Header]               = colors_t::accent_subtle;
            colors[ImGuiCol_HeaderHovered]        = colors_t::control_hover;
            colors[ImGuiCol_HeaderActive]         = colors_t::accent_active;

            colors[ImGuiCol_ResizeGrip]           = colors_t::control_hover;
            colors[ImGuiCol_ResizeGripHovered]    = colors_t::accent_hover;
            colors[ImGuiCol_ResizeGripActive]     = colors_t::accent;

            colors[ImGuiCol_Tab]                  = colors_t::background;
            colors[ImGuiCol_TabHovered]           = colors_t::control_hover;
            colors[ImGuiCol_TabSelected]          = colors_t::panel;
            colors[ImGuiCol_TabSelectedOverline]  = colors_t::accent;
            colors[ImGuiCol_TabDimmed]            = colors_t::background;
            colors[ImGuiCol_TabDimmedSelected]    = colors_t::panel;
            colors[ImGuiCol_DockingPreview]       = colors_t::accent_subtle;

            colors[ImGuiCol_TextLink]             = colors_t::accent;
            colors[ImGuiCol_NavCursor]            = colors_t::accent;
        }
    }

    void editor_theme_t::apply()
    {
        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();

        apply_font();
        apply_metrics(style);
        apply_colors(style);
    }
}

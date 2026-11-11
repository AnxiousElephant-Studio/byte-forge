#include <byte-forge-editor/ui/debug_ui_context.h>
#include <byte-forge-editor/ui/utils/editor_theme.h>

#include <byte-forge-engine/modules/logging/log_macros.h>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>

#include <filesystem>
#include <system_error>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>

namespace byteforge::editor
{
    namespace
    {
        const char* const glsl_version     = "#version 460";
        const char* const layout_file_name = "imgui.ini";

        const char* const initialization_error_format = "Could not initialize the debug UI (ImGui).";
    }

    bool debug_ui_context_t::initialize(const platform::window_t& window)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable;

        std::error_code directory_error;
        layout_file_path = (std::filesystem::current_path(directory_error) / layout_file_name).generic_string();
        ImGui::GetIO().IniFilename = layout_file_path.c_str();

        editor_theme_t::apply();

        if (!ImGui_ImplSDL3_InitForOpenGL(window.sdl_window, SDL_GL_GetCurrentContext()) || !ImGui_ImplOpenGL3_Init(glsl_version))
        {
            LOG_FATAL(initialization_error_format);
            ImGui::DestroyContext();
            return false;
        }

        initialized = true;

        return true;
    }

    void debug_ui_context_t::process_event(const void* native_event) const
    {
        ImGui_ImplSDL3_ProcessEvent(static_cast<const SDL_Event*>(native_event));
    }

    void debug_ui_context_t::begin_frame() const
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
    }

    void debug_ui_context_t::end_frame() const
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void debug_ui_context_t::dispose()
    {
        if (!initialized)
        {
            return;
        }

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();

        initialized = false;
    }
}

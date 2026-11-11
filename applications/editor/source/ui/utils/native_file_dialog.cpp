#include <byte-forge-editor/ui/utils/native_file_dialog.h>

#include <SDL3/SDL_dialog.h>

namespace byteforge::editor
{
    namespace
    {
        const SDL_DialogFileFilter project_file_filters[] =
        {
            { "ByteForge project", "bfproject" },
        };
    }

    void native_file_dialog_t::request_folder(SDL_Window* window)
    {
        SDL_ShowOpenFolderDialog(&native_file_dialog_t::on_selection, this, window, nullptr, false);
    }

    void native_file_dialog_t::request_project_file(SDL_Window* window)
    {
        SDL_ShowOpenFileDialog(&native_file_dialog_t::on_selection, this, window, project_file_filters, 1, nullptr, false);
    }

    bool native_file_dialog_t::take_selected_path(std::string& out_path)
    {
        const std::lock_guard<std::mutex> lock(selection_mutex);

        if (!has_selection)
        {
            return false;
        }

        out_path      = selected_path;
        has_selection = false;

        return true;
    }

    void native_file_dialog_t::on_selection(void* user_data, const char* const* file_list, int filter)
    {
        (void)filter;

        if (file_list == nullptr || file_list[0] == nullptr)
        {
            return;
        }

        native_file_dialog_t* dialog = static_cast<native_file_dialog_t*>(user_data);

        const std::lock_guard<std::mutex> lock(dialog->selection_mutex);

        dialog->selected_path = file_list[0];
        dialog->has_selection = true;
    }
}

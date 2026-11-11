#pragma once

#include <mutex>
#include <string>

struct SDL_Window;

namespace byteforge::editor
{
    class native_file_dialog_t
    {
    public:
        void request_folder(SDL_Window* window);
        void request_project_file(SDL_Window* window);

        bool take_selected_path(std::string& out_path);

    private:
        static void on_selection(void* user_data, const char* const* file_list, int filter);

        std::mutex selection_mutex;
        std::string selected_path;
        bool has_selection = false;
    };
}

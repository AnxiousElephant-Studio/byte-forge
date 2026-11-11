#include <byte-forge-engine/modules/io/file_system.h>
#include <byte-forge-engine/modules/io/file.h>
#include <byte-forge-engine/modules/io/e_file_data_types.h>

#include <algorithm>
#include <filesystem>
#include <system_error>

namespace byteforge::io
{
    namespace
    {
        bool compare_entries(const directory_entry_t& left, const directory_entry_t& right)
        {
            if (left.is_directory != right.is_directory)
            {
                return left.is_directory;
            }

            return left.name < right.name;
        }
    }

    bool file_system_t::exists(const char* path)
    {
        std::error_code error;
        return std::filesystem::exists(path, error);
    }

    bool file_system_t::is_directory(const char* path)
    {
        std::error_code error;
        return std::filesystem::is_directory(path, error);
    }

    bool file_system_t::create_directories(const char* path)
    {
        std::error_code error;
        std::filesystem::create_directories(path, error);

        return !error && is_directory(path);
    }

    bool file_system_t::remove(const char* path)
    {
        std::error_code error;
        return std::filesystem::remove_all(path, error) > 0 && !error;
    }

    bool file_system_t::rename(const char* path, const char* new_path)
    {
        std::error_code error;
        std::filesystem::rename(path, new_path, error);

        return !error;
    }

    i64 file_system_t::get_last_write_time(const char* path)
    {
        std::error_code error;
        const std::filesystem::file_time_type write_time = std::filesystem::last_write_time(path, error);

        return error ? 0 : static_cast<i64>(write_time.time_since_epoch().count());
    }

    bool file_system_t::list_directory(const char* path, std::vector<directory_entry_t>& out_entries)
    {
        out_entries.clear();

        std::error_code error;
        std::filesystem::directory_iterator iterator(path, error);

        if (error)
        {
            return false;
        }

        for (const std::filesystem::directory_entry& entry : iterator)
        {
            directory_entry_t directory_entry;
            directory_entry.name         = entry.path().filename().string();
            directory_entry.path         = entry.path().generic_string();
            directory_entry.extension    = entry.path().extension().string();
            directory_entry.is_directory = entry.is_directory(error);

            out_entries.push_back(directory_entry);
        }

        std::sort(out_entries.begin(), out_entries.end(), compare_entries);

        return true;
    }

    std::string file_system_t::get_parent_path(const char* path)
    {
        return std::filesystem::path(path).parent_path().generic_string();
    }

    std::string file_system_t::get_file_name(const char* path)
    {
        return std::filesystem::path(path).filename().string();
    }

    std::string file_system_t::get_extension(const char* path)
    {
        return std::filesystem::path(path).extension().string();
    }

    std::string file_system_t::join(const char* base_path, const char* relative_path)
    {
        return (std::filesystem::path(base_path) / relative_path).generic_string();
    }

    bool file_system_t::read_text(const char* path, std::string& out_text)
    {
        file_t file;

        if (!file.read(path, e_file_data_types::Text))
        {
            return false;
        }

        out_text.assign(file.content, file.size);

        return true;
    }

    bool file_system_t::write_text(const char* path, const std::string& text)
    {
        file_t file;
        return file.write(path, text.data(), text.size());
    }
}

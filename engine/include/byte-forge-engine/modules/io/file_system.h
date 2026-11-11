#pragma once

#include <byte-forge-engine/modules/io/io_api.h>
#include <byte-forge-engine/api/types.h>

#include <string>
#include <vector>

namespace byteforge::io
{
    struct directory_entry_t
    {
    public:
        std::string name;
        std::string path;
        std::string extension;
        bool is_directory;
    };

    struct BF_IO_API file_system_t
    {
    public:
        static bool exists(const char* path);
        static bool is_directory(const char* path);

        static bool create_directories(const char* path);
        static bool remove(const char* path);
        static bool rename(const char* path, const char* new_path);
        static i64 get_last_write_time(const char* path);
        static bool list_directory(const char* path, std::vector<directory_entry_t>& out_entries);

        static bool read_text(const char* path, std::string& out_text);
        static bool write_text(const char* path, const std::string& text);

        static std::string get_parent_path(const char* path);
        static std::string get_file_name(const char* path);
        static std::string get_extension(const char* path);
        static std::string join(const char* base_path, const char* relative_path);
    };
}

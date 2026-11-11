#pragma once

#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/io/e_file_access_modes.h>
#include <byte-forge-engine/modules/io/e_file_data_types.h>

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <system_error>

namespace byteforge::io
{
    struct file_t
    {
    public:
        char* content;
        usize size;

        file_t() : content(nullptr), size(0) {}

        ~file_t()
        {
            dispose();
        }

        file_t(const file_t&)            = delete;
        file_t& operator=(const file_t&) = delete;

        bool read(const char* path, const e_file_data_types data_type)
        {
            dispose();

            std::error_code error;
            const std::uintmax_t file_size = std::filesystem::file_size(path, error);

            if (error)
            {
                return false;
            }

            FILE* file = std::fopen(path, get_file_mode(e_file_access_modes::Read));

            if (file == nullptr)
            {
                return false;
            }

            size = static_cast<usize>(file_size);

            content = static_cast<char*>(std::malloc(size + 1));

            if (content == nullptr)
            {
                size = 0;
                std::fclose(file);
                return false;
            }

            const usize read_size = std::fread(content, sizeof(char), size, file);

            std::fclose(file);

            if (read_size != size)
            {
                dispose();
                return false;
            }

            if (data_type == e_file_data_types::Text)
            {
                content[size] = '\0';
            }

            return true;
        }

        bool write(const char* path, const void* data, usize data_size)
        {
            FILE* file = std::fopen(path, get_file_mode(e_file_access_modes::Write));

            if (file == nullptr)
            {
                return false;
            }

            const usize written_data_size = std::fwrite(data, sizeof(char), data_size, file);

            std::fclose(file);

            return written_data_size == data_size;
        }

        void dispose()
        {
            if (content != nullptr)
            {
                std::free(content);
                content = nullptr;
            }

            size = 0;
        }

    private:
        constexpr static const char* read_mode  = "rb";
        constexpr static const char* write_mode = "wb";

        constexpr static const char* get_file_mode(const e_file_access_modes access_mode)
        {
            return access_mode == e_file_access_modes::Read ? read_mode : write_mode;
        }
    };
}

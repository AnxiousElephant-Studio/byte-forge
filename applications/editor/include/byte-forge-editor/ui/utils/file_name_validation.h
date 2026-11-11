#pragma once

#include <cstring>

namespace byteforge::editor
{
    inline bool is_ascii_text(const char* text)
    {
        for (const char* character = text; *character != '\0'; character++)
        {
            if (static_cast<unsigned char>(*character) > 127)
            {
                return false;
            }
        }

        return true;
    }

    inline bool has_invalid_file_name_characters(const char* name)
    {
        return std::strpbrk(name, "<>:\"/\\|?*") != nullptr;
    }
}

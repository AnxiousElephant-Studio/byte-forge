#pragma once

#include <byte-forge-engine/api/types.h>

#include <cstring>

namespace byteforge::components
{
    constexpr usize TAGS_MAX_LENGTH = 128;
    constexpr char TAG_SEPARATOR    = ',';

    struct tags_component_t
    {
        char tags[TAGS_MAX_LENGTH];

        bool has_tag(const char* tag) const
        {
            const usize tag_length = std::strlen(tag);

            if (tag_length == 0)
            {
                return false;
            }

            const char* cursor = tags;

            while (*cursor != '\0')
            {
                while (*cursor == TAG_SEPARATOR || *cursor == ' ')
                {
                    cursor++;
                }

                const char* tag_end = cursor;

                while (*tag_end != '\0' && *tag_end != TAG_SEPARATOR)
                {
                    tag_end++;
                }

                const char* trimmed_end = tag_end;

                while (trimmed_end > cursor && trimmed_end[-1] == ' ')
                {
                    trimmed_end--;
                }

                if (static_cast<usize>(trimmed_end - cursor) == tag_length && std::strncmp(cursor, tag, tag_length) == 0)
                {
                    return true;
                }

                cursor = tag_end;
            }

            return false;
        }
    };
}

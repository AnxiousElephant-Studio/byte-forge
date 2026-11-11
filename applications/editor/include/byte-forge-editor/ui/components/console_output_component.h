#pragma once

#include <byte-forge-editor/editor_application_context.h>

#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/logging/e_log_levels.h>

namespace byteforge::editor
{
    class console_output_component_t
    {
    public:
        console_output_component_t()
            : minimum_displayed_log_level(logging::e_log_levels::Info)
            , is_auto_scroll_enabled(true)
            , search_query{}
            , last_seen_total_log_count(0)
        {
        }

        void draw(editor_application_context_t& context);

    private:
        static constexpr usize search_query_max_length = 128;

        logging::e_log_levels minimum_displayed_log_level;
        bool is_auto_scroll_enabled;
        char search_query[search_query_max_length];
        u64 last_seen_total_log_count;
    };
}

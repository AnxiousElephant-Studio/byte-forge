#pragma once

#include <byte-forge-engine/modules/logging/logging_api.h>
#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/logging/e_log_levels.h>
#include <byte-forge-engine/modules/logging/log_buffer.h>
#include <byte-forge-engine/modules/logging/log_sink.h>

namespace byteforge::logging
{
    struct BF_LOGGING_API logger_t
    {
        static void set_sink(log_sink_t sink);
        static void set_min_level(e_log_levels level);
        static void set_console_output(bool enabled);

        static e_log_levels get_min_level();
        static log_buffer_t& get_buffer();

        static void standard_log(e_log_levels level, const char* format, ...);

        static void detailed_log(e_log_levels level, const char* file_name, i32 code_line, const char* function_name, const char* format, ...);
    };
}

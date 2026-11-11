#include <byte-forge-engine/modules/logging/logger.h>

#include <modules/logging/log_level_info.h>
#include <modules/logging/log_styles.h>

#include <cstdarg>
#include <cstdio>
#include <ctime>

namespace byteforge::logging
{
    namespace
    {
        const usize datetime_string_buffer_size = 20;
        const usize message_buffer_size         = 1024;
        const usize location_buffer_size        = 256;
        const usize line_buffer_size            = 1400;

        const i32 date_year_offset  = 1900;
        const i32 date_month_offset = 1;

        const char* const datetime_string_unknown = "unknown-datetime";
        const char* const datetime_output_format  = "%02d-%02d-%04d %02d:%02d:%02d";
        const char* const location_output_format  = " [%s:%d:%s]";
        const char* const line_output_format      = "[%s] [%s]%s %s";
        const char* const stream_output_format    = "%s%s%s\n";

        log_buffer_t buffer;
        log_sink_t current_sink = nullptr;
        e_log_levels min_level = e_log_levels::Info;
        bool console_output_enabled = true;

        bool get_local_time(const std::time_t& now, std::tm& out_local_time)
        {
#if defined(_WIN32)
            return localtime_s(&out_local_time, &now) == 0;
#else
            return localtime_r(&now, &out_local_time) != nullptr;
#endif
        }

        void get_current_datetime_string(char* datetime_buffer, usize datetime_buffer_size)
        {
            std::time_t now = std::time(nullptr);
            std::tm local_time = {};

            if (!get_local_time(now, local_time))
            {
                std::snprintf(datetime_buffer, datetime_buffer_size, "%s", datetime_string_unknown);
                return;
            }

            std::snprintf(datetime_buffer, datetime_buffer_size, datetime_output_format,
                local_time.tm_mday, local_time.tm_mon + date_month_offset, local_time.tm_year + date_year_offset,
                local_time.tm_hour, local_time.tm_min, local_time.tm_sec);
        }

        bool should_log(e_log_levels level)
        {
            return level >= min_level;
        }

        void write(e_log_levels level, const char* location, const char* message)
        {
            char datetime[datetime_string_buffer_size];
            get_current_datetime_string(datetime, sizeof(datetime));

            char line[line_buffer_size];
            std::snprintf(line, sizeof(line), line_output_format, datetime, log_level_info_t::get_name(level), location, message);

            const log_entry_t entry = { level, line };

            buffer.add(entry);

            if (current_sink != nullptr)
            {
                current_sink(entry);
            }

            if (console_output_enabled)
            {
                std::FILE* output_stream = level >= e_log_levels::Warning ? stderr : stdout;

                std::fprintf(output_stream, stream_output_format, log_level_info_t::get_color(level), line, log_style_t::reset);
            }
        }
    }

    void logger_t::set_sink(log_sink_t sink)
    {
        current_sink = sink;
    }

    void logger_t::set_min_level(e_log_levels level)
    {
        min_level = level;
    }

    e_log_levels logger_t::get_min_level()
    {
        return min_level;
    }

    void logger_t::set_console_output(bool enabled)
    {
        console_output_enabled = enabled;
    }

    log_buffer_t& logger_t::get_buffer()
    {
        return buffer;
    }

    void logger_t::standard_log(e_log_levels level, const char* format, ...)
    {
        if (!should_log(level))
        {
            return;
        }

        char message[message_buffer_size];

        std::va_list arguments;
        va_start(arguments, format);
        std::vsnprintf(message, sizeof(message), format, arguments);
        va_end(arguments);

        write(level, "", message);
    }

    void logger_t::detailed_log(e_log_levels level, const char* file_name, i32 code_line, const char* function_name, const char* format, ...)
    {
        if (!should_log(level))
        {
            return;
        }

        char message[message_buffer_size];

        std::va_list arguments;
        va_start(arguments, format);
        std::vsnprintf(message, sizeof(message), format, arguments);
        va_end(arguments);

        char location[location_buffer_size];
        std::snprintf(location, sizeof(location), location_output_format, file_name, code_line, function_name);

        write(level, location, message);
    }
}

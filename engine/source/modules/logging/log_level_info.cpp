#include <modules/logging/log_level_info.h>
#include <modules/logging/log_styles.h>

namespace byteforge::logging
{
    const char* log_level_info_t::get_color(e_log_levels level)
    {
        switch (level)
        {
            case e_log_levels::Info:    return log_style_t::white_text;
            case e_log_levels::Debug:   return log_style_t::cyan_text;
            case e_log_levels::Trace:   return log_style_t::blue_text;
            case e_log_levels::Success: return log_style_t::green_text;
            case e_log_levels::Warning: return log_style_t::yellow_text;
            case e_log_levels::Error:   return log_style_t::red_text;
            case e_log_levels::Fatal:   return log_style_t::red_background;
        }

        return log_style_t::reset;
    }

    const char* log_level_info_t::get_name(e_log_levels level)
    {
        switch (level)
        {
            case e_log_levels::Info:    return "INFO";
            case e_log_levels::Debug:   return "DEBUG";
            case e_log_levels::Trace:   return "TRACE";
            case e_log_levels::Success: return "SUCCESS";
            case e_log_levels::Warning: return "WARNING";
            case e_log_levels::Error:   return "ERROR";
            case e_log_levels::Fatal:   return "FATAL";
        }

        return "UNKNOWN";
    }
}

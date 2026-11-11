#pragma once

#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/logging/logger.h>

#include <string_view>

namespace byteforge::logging
{
    constexpr const char* extract_file_name(std::string_view path)
    {
        usize position = path.find_last_of("\\/");

        return position == std::string_view::npos ? path.data() : path.data() + position + 1;
    }
}

#define BF_LOG_FILE_NAME byteforge::logging::extract_file_name(__FILE__)

#define LOG_FATAL(...)   byteforge::logging::logger_t::detailed_log(byteforge::logging::e_log_levels::Fatal,   BF_LOG_FILE_NAME, __LINE__, __func__, __VA_ARGS__)
#define LOG_ERROR(...)   byteforge::logging::logger_t::detailed_log(byteforge::logging::e_log_levels::Error,   BF_LOG_FILE_NAME, __LINE__, __func__, __VA_ARGS__)
#define LOG_WARNING(...) byteforge::logging::logger_t::detailed_log(byteforge::logging::e_log_levels::Warning, BF_LOG_FILE_NAME, __LINE__, __func__, __VA_ARGS__)
#define LOG_TRACE(...)   byteforge::logging::logger_t::detailed_log(byteforge::logging::e_log_levels::Trace,   BF_LOG_FILE_NAME, __LINE__, __func__, __VA_ARGS__)

#define LOG_SUCCESS(...) byteforge::logging::logger_t::standard_log(byteforge::logging::e_log_levels::Success, __VA_ARGS__)
#define LOG_DEBUG(...)   byteforge::logging::logger_t::standard_log(byteforge::logging::e_log_levels::Debug,   __VA_ARGS__)
#define LOG_INFO(...)    byteforge::logging::logger_t::standard_log(byteforge::logging::e_log_levels::Info,    __VA_ARGS__)

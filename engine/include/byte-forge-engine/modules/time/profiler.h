#pragma once

#include <byte-forge-engine/modules/time/time_api.h>
#include <byte-forge-engine/api/types.h>

#include <chrono>
#include <string>
#include <vector>

namespace byteforge::time
{
    struct profiler_sample_t
    {
    public:
        std::string name;
        std::chrono::steady_clock::time_point start;
        f64 last_ms;
        f64 average_ms;
    };

    class BF_TIME_API profiler_t
    {
    public:
        profiler_t() : samples(), enabled(false) {}

        void set_enabled(const bool value);

        void begin(const char* name);
        void end(const char* name);

        bool is_enabled() const;
        usize get_sample_count() const;
        const profiler_sample_t& get_sample(const usize index) const;
        const profiler_sample_t* find(const char* name) const;

    private:
        profiler_sample_t* find_or_create(const char* name);

        std::vector<profiler_sample_t> samples;
        bool enabled;
    };

    struct scoped_profile_t
    {
    public:
        scoped_profile_t(profiler_t* profiler, const char* name) : profiler(profiler != nullptr && profiler->is_enabled() ? profiler : nullptr), name(name)
        {
            if (this->profiler != nullptr)
            {
                this->profiler->begin(name);
            }
        }

        ~scoped_profile_t()
        {
            if (profiler != nullptr)
            {
                profiler->end(name);
            }
        }

        scoped_profile_t(const scoped_profile_t&)            = delete;
        scoped_profile_t& operator=(const scoped_profile_t&) = delete;

    private:
        profiler_t* profiler;
        const char* name;
    };
}

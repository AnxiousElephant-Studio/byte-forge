#include <byte-forge-engine/modules/time/profiler.h>

namespace byteforge::time
{
    namespace
    {
        const f64 average_smoothing = 0.9;
    }

    void profiler_t::set_enabled(const bool value)
    {
        if (enabled != value)
        {
            samples.clear();
        }

        enabled = value;
    }

    void profiler_t::begin(const char* name)
    {
        if (!enabled)
        {
            return;
        }

        find_or_create(name)->start = std::chrono::steady_clock::now();
    }

    void profiler_t::end(const char* name)
    {
        if (!enabled)
        {
            return;
        }

        profiler_sample_t* sample = find_or_create(name);

        sample->last_ms    = std::chrono::duration<f64, std::milli>(std::chrono::steady_clock::now() - sample->start).count();
        sample->average_ms = sample->average_ms == 0.0 ? sample->last_ms : sample->average_ms * average_smoothing + sample->last_ms * (1.0 - average_smoothing);
    }

    bool profiler_t::is_enabled() const
    {
        return enabled;
    }

    usize profiler_t::get_sample_count() const
    {
        return samples.size();
    }

    const profiler_sample_t& profiler_t::get_sample(const usize index) const
    {
        return samples[index];
    }

    const profiler_sample_t* profiler_t::find(const char* name) const
    {
        for (usize index = 0; index < samples.size(); index++)
        {
            if (samples[index].name == name)
            {
                return &samples[index];
            }
        }

        return nullptr;
    }

    profiler_sample_t* profiler_t::find_or_create(const char* name)
    {
        for (usize index = 0; index < samples.size(); index++)
        {
            if (samples[index].name == name)
            {
                return &samples[index];
            }
        }

        samples.push_back({ name, std::chrono::steady_clock::now(), 0.0, 0.0 });

        return &samples.back();
    }
}

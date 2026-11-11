#include <byte-forge-engine/modules/logging/log_buffer.h>

namespace byteforge::logging
{
    void log_buffer_t::add(const log_entry_t& entry)
    {
        total_count++;

        if (entries.size() < capacity)
        {
            entries.push_back(entry);
            return;
        }

        entries[oldest_index] = entry;
        oldest_index = (oldest_index + 1) % capacity;
    }

    void log_buffer_t::clear()
    {
        entries.clear();
        oldest_index = 0;
    }

    usize log_buffer_t::get_count() const
    {
        return entries.size();
    }

    const log_entry_t& log_buffer_t::get(const usize index) const
    {
        return entries[(oldest_index + index) % entries.size()];
    }

    u64 log_buffer_t::get_total_count() const
    {
        return total_count;
    }
}

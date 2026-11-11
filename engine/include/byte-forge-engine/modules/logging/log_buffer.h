#pragma once

#include <byte-forge-engine/modules/logging/logging_api.h>
#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/logging/log_entry.h>

#include <vector>

namespace byteforge::logging
{
    class BF_LOGGING_API log_buffer_t
    {
    public:
        log_buffer_t() : entries(), capacity(DEFAULT_CAPACITY), oldest_index(0), total_count(0) {}

        void add(const log_entry_t& entry);
        void clear();

        const log_entry_t& get(const usize index) const;
        usize get_count() const;
        u64 get_total_count() const;

    private:
        static constexpr usize DEFAULT_CAPACITY = 2000;

        std::vector<log_entry_t> entries;
        usize capacity;
        usize oldest_index;
        u64 total_count;
    };
}

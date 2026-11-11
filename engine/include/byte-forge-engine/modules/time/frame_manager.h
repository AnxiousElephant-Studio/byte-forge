#pragma once

#include <byte-forge-engine/modules/time/time_api.h>
#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/time/frame_settings.h>

namespace byteforge::time
{
    struct BF_TIME_API frame_manager_t
    {
    public:
        f64 delta_time;

        frame_manager_t()
            : delta_time(0.0)
            , first_frame_tick_count(0)
            , last_frame_tick_count(0)
            , current_frame_start_tick_count(0)
            , current_frame_end_tick_count(0)
            , target_frame_time_in_ms(0.0)
        {
        }

        void reset(const frame_settings_t& frame_settings);

        void start_frame();
        void end_frame();

        f64 get_elapsed_time() const;

    private:
        u64 first_frame_tick_count;
        u64 last_frame_tick_count;
        u64 current_frame_start_tick_count;
        u64 current_frame_end_tick_count;
        f64 target_frame_time_in_ms;
    };
}

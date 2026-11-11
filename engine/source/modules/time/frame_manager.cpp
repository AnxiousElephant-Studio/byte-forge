#include <byte-forge-engine/modules/time/frame_manager.h>
#include <byte-forge-engine/modules/time/time_converter.h>

#include <SDL3/SDL_timer.h>

#include <algorithm>

namespace byteforge::time
{
    namespace
    {
        const f64 max_delta_time = 0.05;
    }

    void frame_manager_t::reset(const frame_settings_t& frame_settings)
    {
        first_frame_tick_count         = SDL_GetTicks();
        last_frame_tick_count          = first_frame_tick_count;
        current_frame_start_tick_count = 0;
        current_frame_end_tick_count   = 0;
        delta_time                     = 0.0;
        target_frame_time_in_ms        = time_converter_t::sec_to_ms(1.0 / frame_settings.target_fps);
    }

    void frame_manager_t::start_frame()
    {
        current_frame_start_tick_count = SDL_GetTicks();
    }

    void frame_manager_t::end_frame()
    {
        current_frame_end_tick_count = SDL_GetTicks();
        delta_time = std::min(time_converter_t::ms_to_sec(static_cast<f64>(current_frame_end_tick_count - last_frame_tick_count)), max_delta_time);
        last_frame_tick_count = current_frame_end_tick_count;

        const f64 frame_duration = static_cast<f64>(current_frame_end_tick_count - current_frame_start_tick_count);

        if (frame_duration < target_frame_time_in_ms)
        {
            SDL_Delay(static_cast<u32>(target_frame_time_in_ms - frame_duration));
        }
    }

    f64 frame_manager_t::get_elapsed_time() const
    {
        if (current_frame_start_tick_count < first_frame_tick_count)
        {
            return 0.0;
        }

        return time_converter_t::ms_to_sec(static_cast<f64>(current_frame_start_tick_count - first_frame_tick_count));
    }
}

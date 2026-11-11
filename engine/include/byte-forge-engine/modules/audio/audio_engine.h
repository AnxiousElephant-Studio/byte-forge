#pragma once

#include <byte-forge-engine/modules/audio/audio_api.h>
#include <byte-forge-engine/api/types.h>

#include <unordered_map>

struct ma_engine;
struct ma_sound;

namespace byteforge::audio
{
    class BF_AUDIO_API audio_engine_t
    {
    public:
        audio_engine_t() : internal_engine(nullptr), sounds_by_handle(), next_handle(1) {}

        audio_engine_t(const audio_engine_t&)            = delete;
        audio_engine_t& operator=(const audio_engine_t&) = delete;

        bool initialize();
        void dispose();

        u32 play(const char* path, const f32 volume, const bool loop);
        void stop(const u32 handle);
        void stop_all();

        void update();

        bool is_playing(const u32 handle) const;
        usize get_active_count() const;

    private:
        ma_engine* internal_engine;
        std::unordered_map<u32, ma_sound*> sounds_by_handle;
        u32 next_handle;
    };
}

#include <byte-forge-engine/modules/audio/audio_engine.h>
#include <byte-forge-engine/modules/logging/log_macros.h>

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#include <vector>

namespace byteforge::audio
{
    namespace
    {
        const u32 invalid_handle = 0;

        const char* const initialization_error_format = "Could not initialize the audio engine. Error code: %d.";
        const char* const load_error_format           = "Could not load audio at path: '%s'. Error code: %d.";
    }

    bool audio_engine_t::initialize()
    {
        internal_engine = new ma_engine();

        const ma_result result_code = ma_engine_init(nullptr, internal_engine);

        if (result_code != MA_SUCCESS)
        {
            LOG_ERROR(initialization_error_format, static_cast<i32>(result_code));
            dispose();
            return false;
        }

        return true;
    }

    u32 audio_engine_t::play(const char* path, const f32 volume, const bool loop)
    {
        if (internal_engine == nullptr || path == nullptr || path[0] == '\0')
        {
            return invalid_handle;
        }

        ma_sound* sound = new ma_sound();
        const ma_result result = ma_sound_init_from_file(internal_engine, path, 0, nullptr, nullptr, sound);

        if (result != MA_SUCCESS)
        {
            LOG_ERROR(load_error_format, path, static_cast<i32>(result));
            delete sound;
            return invalid_handle;
        }

        ma_sound_set_volume(sound, volume);
        ma_sound_set_looping(sound, loop ? MA_TRUE : MA_FALSE);
        ma_sound_start(sound);

        const u32 handle = next_handle++;
        sounds_by_handle[handle] = sound;

        return handle;
    }

    void audio_engine_t::stop(const u32 handle)
    {
        std::unordered_map<u32, ma_sound*>::iterator iterator = sounds_by_handle.find(handle);

        if (iterator == sounds_by_handle.end())
        {
            return;
        }

        ma_sound_uninit(iterator->second);
        delete iterator->second;
        sounds_by_handle.erase(iterator);
    }

    void audio_engine_t::stop_all()
    {
        for (std::pair<const u32, ma_sound*>& entry : sounds_by_handle)
        {
            ma_sound_uninit(entry.second);
            delete entry.second;
        }

        sounds_by_handle.clear();
    }

    bool audio_engine_t::is_playing(const u32 handle) const
    {
        std::unordered_map<u32, ma_sound*>::const_iterator iterator = sounds_by_handle.find(handle);

        return iterator != sounds_by_handle.end() && ma_sound_is_playing(iterator->second) == MA_TRUE;
    }

    void audio_engine_t::update()
    {
        std::vector<u32> finished_handles;

        for (const std::pair<const u32, ma_sound*>& entry : sounds_by_handle)
        {
            if (ma_sound_at_end(entry.second) == MA_TRUE)
            {
                finished_handles.push_back(entry.first);
            }
        }

        for (usize index = 0; index < finished_handles.size(); index++)
        {
            stop(finished_handles[index]);
        }
    }

    usize audio_engine_t::get_active_count() const
    {
        return sounds_by_handle.size();
    }

    void audio_engine_t::dispose()
    {
        stop_all();

        if (internal_engine != nullptr)
        {
            ma_engine_uninit(internal_engine);
            delete internal_engine;
            internal_engine = nullptr;
        }
    }
}

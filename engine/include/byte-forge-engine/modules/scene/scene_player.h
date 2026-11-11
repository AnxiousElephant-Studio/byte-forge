#pragma once

#include <byte-forge-engine/modules/scene/scene_api.h>
#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/audio/audio_engine.h>
#include <byte-forge-engine/modules/ecs/e_system_stages.h>
#include <byte-forge-engine/modules/scene/scene.h>
#include <byte-forge-engine/modules/scene/scene_script_system.h>
#include <byte-forge-engine/modules/time/profiler.h>

namespace byteforge::scene
{
    class BF_SCENE_API scene_player_t
    {
    public:
        scene_script_system_t script_system;
        time::profiler_t* profiler;
        f32 time_scale;

        scene_player_t() : script_system(), profiler(nullptr), time_scale(1.0f), scene(nullptr), audio_engine(nullptr), input_state(nullptr), running(false), paused(false), step_requested(false) {}

        bool initialize(scripting::scripting_context_t& scripting_context, scene_t& scene, audio::audio_engine_t& audio_engine, platform::input_state_t& input_state);
        void dispose();

        void start();
        void stop();
        void set_paused(const bool value);
        void request_step();

        void update(const f64 delta_time);

        bool is_running() const { return running; }
        bool is_paused() const { return paused; }
        audio::audio_engine_t* get_audio_engine() const { return audio_engine; }

    private:
        void run_system_stage(const ecs::e_system_stages stage, const f64 delta_time);
        void destroy_queued_entities();

        scene_t* scene;
        audio::audio_engine_t* audio_engine;
        platform::input_state_t* input_state;
        bool running;
        bool paused;
        bool step_requested;
    };
}

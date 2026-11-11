#include <byte-forge-engine/modules/scene/scene_player.h>

namespace byteforge::scene
{
    namespace
    {
        const f64 step_delta_time = 1.0 / 60.0;
    }

    bool scene_player_t::initialize(scripting::scripting_context_t& scripting_context, scene_t& scene, audio::audio_engine_t& audio_engine, platform::input_state_t& input_state)
    {
        this->scene        = &scene;
        this->audio_engine = &audio_engine;
        this->input_state  = &input_state;

        return script_system.initialize(scripting_context, scene, audio_engine, input_state);
    }

    void scene_player_t::dispose()
    {
        if (scene != nullptr)
        {
            stop();
        }

        script_system.dispose();
        scene        = nullptr;
        audio_engine = nullptr;
        input_state  = nullptr;
    }

    void scene_player_t::start()
    {
        stop();

        input_state->clear();
        input_state->enabled = true;

        run_system_stage(ecs::e_system_stages::Start, 0.0);
        destroy_queued_entities();
        running = true;
    }

    void scene_player_t::stop()
    {
        run_system_stage(ecs::e_system_stages::Stop, 0.0);
        audio_engine->stop_all();

        input_state->enabled = false;
        input_state->clear();

        running        = false;
        paused         = false;
        step_requested = false;
    }

    void scene_player_t::set_paused(const bool value)
    {
        paused = value;
    }

    void scene_player_t::request_step()
    {
        if (running && paused)
        {
            step_requested = true;
        }
    }

    void scene_player_t::update(const f64 delta_time)
    {
        if (!running)
        {
            return;
        }

        const bool stepping = step_requested;
        step_requested = false;

        if (paused && !stepping)
        {
            return;
        }

        const f64 scaled_delta_time = (stepping ? step_delta_time : delta_time) * static_cast<f64>(time_scale);

        run_system_stage(ecs::e_system_stages::Update, scaled_delta_time);
        destroy_queued_entities();
    }

    void scene_player_t::destroy_queued_entities()
    {
        script_system.release_entities(scene->destroy_queued_entities());
    }

    void scene_player_t::run_system_stage(const ecs::e_system_stages stage, const f64 delta_time)
    {
        std::vector<ecs::system_t*>& systems = scene->registry.systems;

        for (usize index = 0; index < systems.size(); index++)
        {
            ecs::system_t& system = *systems[index];

            if (system.stage != stage || !system.is_enabled || system.update_function == nullptr)
            {
                continue;
            }

            time::scoped_profile_t system_profile(profiler, system.name.c_str());
            system.update_function(scene->registry, system, delta_time);
        }
    }
}

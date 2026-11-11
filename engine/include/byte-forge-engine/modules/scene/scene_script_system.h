#pragma once

#include <byte-forge-engine/modules/scene/scene_api.h>
#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/audio/audio_engine.h>
#include <byte-forge-engine/modules/platform/input/input_state.h>
#include <byte-forge-engine/modules/scene/scene.h>
#include <byte-forge-engine/modules/scripting/scripting_context.h>

#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct lua_State;

namespace byteforge::scene
{
    using stamp_loader_t = std::function<bool(scene_t& scene, const char* path, ecs::entity_t& out_root, std::string& out_error)>;

    class BF_SCENE_API scene_script_system_t
    {
    public:
        stamp_loader_t stamp_loader;

        scene_script_system_t()
            : stamp_loader()
            , lua_state(nullptr)
            , scene(nullptr)
            , audio_engine(nullptr)
            , input_state(nullptr)
            , running(false)
            , elapsed_time(0.0)
            , delta_time(0.0)
            , requested_level_path()
            , entity_environments()
            , failed_entities()
        {
        }

        bool initialize(scripting::scripting_context_t& scripting_context, scene_t& scene, audio::audio_engine_t& audio_engine, platform::input_state_t& input_state);
        void dispose();

        void start(const std::vector<ecs::entity_t>& entities);
        void stop();

        void update(const std::vector<ecs::entity_t>& entities, const f64 frame_delta_time);
        void notify_collision(const ecs::entity_t entity, const ecs::entity_t other);
        void release_entities(const std::vector<ecs::entity_t>& entities);

        void request_level(const char* level_path);
        bool take_level_request(std::string& out_level_path);

        scene_t& get_scene() const { return *scene; }
        audio::audio_engine_t& get_audio_engine() const { return *audio_engine; }
        platform::input_state_t& get_input_state() const { return *input_state; }
        f64 get_elapsed_time() const { return elapsed_time; }
        f64 get_delta_time() const { return delta_time; }

        bool is_running() const { return running; }
        usize get_environment_count() const { return entity_environments.size(); }

    private:
        bool start_entity(const ecs::entity_t entity);
        bool load_script(const ecs::entity_t entity, const char* script_path);
        bool push_function(const ecs::entity_t entity, const char* function_name);
        void invoke(const ecs::entity_t entity, const char* function_name, const i32 argument_count);
        void register_api();

        lua_State* lua_state;
        scene_t* scene;
        audio::audio_engine_t* audio_engine;
        platform::input_state_t* input_state;
        bool running;
        f64 elapsed_time;
        f64 delta_time;
        std::string requested_level_path;
        std::unordered_map<u64, i32> entity_environments;
        std::unordered_set<u64> failed_entities;
    };
}

#pragma once

#include <byte-forge-engine/modules/scene/scene_api.h>
#include <byte-forge-engine/modules/assets/asset_manager.h>
#include <byte-forge-engine/modules/audio/audio_engine.h>
#include <byte-forge-engine/modules/ecs/system_schedule.h>
#include <byte-forge-engine/modules/math/vectors/vector2i.h>
#include <byte-forge-engine/modules/platform/input/input_state.h>
#include <byte-forge-engine/modules/rendering/sprite/sprite_renderer.h>
#include <byte-forge-engine/modules/scene/scene.h>
#include <byte-forge-engine/modules/scene/scene_player.h>
#include <byte-forge-engine/modules/scene/scene_render_context.h>
#include <byte-forge-engine/modules/scripting/scripting_context.h>

namespace byteforge::scene
{
    class BF_SCENE_API scene_context_t
    {
    public:
        scene_t scene;
        scene_player_t scene_player;
        scene_render_context_t render_context;

        scene_context_t() : scene(), scene_player(), render_context(), system_schedule(), initialized(false) {}

        scene_context_t(const scene_context_t&)            = delete;
        scene_context_t& operator=(const scene_context_t&) = delete;

        bool initialize(scripting::scripting_context_t& scripting_context, audio::audio_engine_t& audio_engine, platform::input_state_t& input_state);
        void dispose();

        bool add_system(const ecs::system_descriptor_t& descriptor);
        bool remove_system(const char* name);
        bool set_system_order(const char* name, const i32 order);

        void set_render_resources(rendering::sprite_renderer_t& sprite_renderer, assets::asset_manager_t& asset_manager);
        void set_render_target_size(const math::vec2i_t& target_size);
        void set_render_target_framebuffer(const rendering::opengl_framebuffer_t* target_framebuffer);
        void set_lighting_resources(rendering::point_light_renderer_t& point_light_renderer, rendering::opengl_framebuffer_t& light_map_framebuffer);

        void update(const f64 delta_time);

        const ecs::system_schedule_t& get_system_schedule() const;
        bool has_system(const char* name) const;

    private:
        ecs::system_schedule_t system_schedule;
        bool initialized;
    };
}

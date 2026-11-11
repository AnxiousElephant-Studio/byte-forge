#include <byte-forge-engine/modules/scene/scene_context.h>

namespace byteforge::scene
{
    bool scene_context_t::initialize(scripting::scripting_context_t& scripting_context, audio::audio_engine_t& audio_engine, platform::input_state_t& input_state)
    {
        if (!scene.initialize() || !scene_player.initialize(scripting_context, scene, audio_engine, input_state))
        {
            return false;
        }

        initialized = true;

        return system_schedule.attach(scene.registry);
    }

    void scene_context_t::dispose()
    {
        scene_player.dispose();
        scene.dispose();
        system_schedule.clear();
        render_context = scene_render_context_t();
        initialized    = false;
    }

    bool scene_context_t::add_system(const ecs::system_descriptor_t& descriptor)
    {
        if (!system_schedule.add_system(descriptor))
        {
            return false;
        }

        if (initialized && !system_schedule.attach_system(scene.registry, descriptor))
        {
            system_schedule.remove_system(descriptor.name.c_str());
            return false;
        }

        return true;
    }

    bool scene_context_t::remove_system(const char* name)
    {
        const bool removed_from_schedule = system_schedule.remove_system(name);
        const bool removed_from_registry = scene.registry.remove_system(name);

        return removed_from_schedule || removed_from_registry;
    }

    bool scene_context_t::set_system_order(const char* name, const i32 order)
    {
        ecs::system_descriptor_t* descriptor = system_schedule.find_system_descriptor(name);

        if (descriptor == nullptr)
        {
            return false;
        }

        descriptor->order = order;

        if (ecs::system_t* system = scene.registry.find_system(name))
        {
            system->order = order;
            scene.registry.sort_systems();
        }

        return true;
    }

    void scene_context_t::set_render_resources(rendering::sprite_renderer_t& sprite_renderer, assets::asset_manager_t& asset_manager)
    {
        render_context.sprite_renderer = &sprite_renderer;
        render_context.asset_manager   = &asset_manager;
    }

    void scene_context_t::set_render_target_size(const math::vec2i_t& target_size)
    {
        render_context.target_size = target_size;
    }

    void scene_context_t::set_render_target_framebuffer(const rendering::opengl_framebuffer_t* target_framebuffer)
    {
        render_context.target_framebuffer = target_framebuffer;
    }

    void scene_context_t::set_lighting_resources(rendering::point_light_renderer_t& point_light_renderer, rendering::opengl_framebuffer_t& light_map_framebuffer)
    {
        render_context.point_light_renderer  = &point_light_renderer;
        render_context.light_map_framebuffer = &light_map_framebuffer;
    }

    void scene_context_t::update(const f64 delta_time)
    {
        scene_player.update(delta_time);
        scene.registry.run_systems(ecs::e_system_stages::Render, delta_time);
    }

    const ecs::system_schedule_t& scene_context_t::get_system_schedule() const
    {
        return system_schedule;
    }

    bool scene_context_t::has_system(const char* name) const
    {
        for (const ecs::system_descriptor_t& descriptor : system_schedule.get_system_descriptors())
        {
            if (descriptor.name == name)
            {
                return true;
            }
        }

        return false;
    }
}

#include <byte-forge-engine/modules/scene/scene_default_2D_systems.h>
#include <byte-forge-engine/modules/scene/scene_context.h>
#include <byte-forge-engine/modules/physics/physics_2D.h>
#include <byte-forge-engine/modules/math/vectors/vector2.h>
#include <byte-forge-engine/modules/rendering/lighting/point_light.h>
#include <byte-forge-engine/modules/rendering/material/material.h>
#include <byte-forge-engine/modules/rendering/opengl/pass/opengl_render_pass.h>
#include <byte-forge-engine/modules/rendering/opengl/state/opengl_render_state.h>

namespace byteforge::scene
{
    namespace
    {
        const char* const audio_start_system_name  = "audio_start";
        const char* const script_start_system_name = "script_start";
        const char* const physics_2D_system_name   = "physics_2D";
        const char* const collision_2D_system_name = "collision_2D";
        const char* const script_system_name       = "script";
        const char* const render_2D_begin_name     = "render_2D_begin";
        const char* const camera_2D_system_name    = "camera_2D";
        const char* const light_2D_system_name     = "light_2D";
        const char* const sprite_2D_system_name    = "sprite_2D";
        const char* const label_2D_system_name     = "label_2D";
        const char* const script_stop_system_name  = "script_stop";

        const f32 default_camera_zoom = 1.0f;

        const char* const default_2D_system_names[] =
        {
            audio_start_system_name, script_start_system_name,
            physics_2D_system_name, collision_2D_system_name, script_system_name,
            render_2D_begin_name, camera_2D_system_name, light_2D_system_name, sprite_2D_system_name, label_2D_system_name,
            script_stop_system_name
        };

        scene_context_t& get_scene_context(ecs::system_t& system)
        {
            return *static_cast<scene_context_t*>(system.data);
        }

        void run_audio_start(ecs::registry_t&, ecs::system_t& system, const f64)
        {
            scene_context_t& scene_context = get_scene_context(system);

            for (usize index = 0; index < system.entities.size(); index++)
            {
                const components::audio_source_component_t* audio_source = scene_context.scene.get_audio_source(system.entities[index]);

                if (audio_source->play_on_start)
                {
                    scene_context.scene_player.get_audio_engine()->play(audio_source->clip_path, audio_source->volume, audio_source->loop);
                }
            }
        }

        void run_script_start(ecs::registry_t&, ecs::system_t& system, const f64)
        {
            get_scene_context(system).scene_player.script_system.start(system.entities);
        }

        void run_physics_2D(ecs::registry_t&, ecs::system_t& system, const f64 delta_time)
        {
            scene_t& scene = get_scene_context(system).scene;

            for (usize index = 0; index < system.entities.size(); index++)
            {
                const ecs::entity_t entity = system.entities[index];

                components::transform_2D_component_t* transform     = scene.get_transform_2D(entity);
                const components::velocity_2D_component_t* velocity = scene.get_velocity_2D(entity);

                transform->position = physics::physics_2D_t::move(transform->position, velocity->velocity, velocity->speed, static_cast<f32>(delta_time));
            }
        }

        void run_collision_2D(ecs::registry_t&, ecs::system_t& system, const f64)
        {
            scene_context_t& scene_context = get_scene_context(system);
            scene_t& scene                 = scene_context.scene;

            const std::vector<ecs::entity_t>& entities = system.entities;

            for (usize first = 0; first < entities.size(); first++)
            {
                const components::transform_2D_component_t first_transform    = scene.get_world_transform_2D(entities[first]);
                const components::box_collider_2D_component_t* first_collider = scene.get_box_collider_2D(entities[first]);

                for (usize second = first + 1; second < entities.size(); second++)
                {
                    const components::transform_2D_component_t second_transform    = scene.get_world_transform_2D(entities[second]);
                    const components::box_collider_2D_component_t* second_collider = scene.get_box_collider_2D(entities[second]);

                    if (physics::physics_2D_t::check_aabb_collision(
                        first_transform.position + first_collider->offset, first_collider->size,
                        second_transform.position + second_collider->offset, second_collider->size))
                    {
                        scene_context.scene_player.script_system.notify_collision(entities[first], entities[second]);
                        scene_context.scene_player.script_system.notify_collision(entities[second], entities[first]);
                    }
                }
            }
        }

        void run_script(ecs::registry_t&, ecs::system_t& system, const f64 delta_time)
        {
            get_scene_context(system).scene_player.script_system.update(system.entities, delta_time);
        }

        void run_script_stop(ecs::registry_t&, ecs::system_t& system, const f64)
        {
            get_scene_context(system).scene_player.script_system.stop();
        }

        void run_render_2D_begin(ecs::registry_t&, ecs::system_t& system, const f64 delta_time)
        {
            scene_render_context_t& render_context = get_scene_context(system).render_context;

            render_context.elapsed_time += delta_time;

            if (!render_context.is_ready())
            {
                return;
            }

            render_context.sprite_renderer->begin(render_context.target_size, math::vec2_t(), default_camera_zoom, false);
            render_context.sprite_renderer->set_material_frame(static_cast<f32>(render_context.elapsed_time), nullptr);
        }

        void bind_render_target(const scene_render_context_t& render_context)
        {
            if (render_context.target_framebuffer != nullptr)
            {
                render_context.target_framebuffer->bind();
                rendering::opengl_render_state_t::set_viewport(render_context.target_framebuffer->get_bounds());
                return;
            }

            rendering::opengl_render_pass_t::begin_window(render_context.target_size);
        }

        void run_light_2D(ecs::registry_t&, ecs::system_t& system, const f64)
        {
            scene_context_t& scene_context        = get_scene_context(system);
            scene_render_context_t& render_context = scene_context.render_context;

            if (!render_context.is_ready() || !render_context.has_lighting())
            {
                return;
            }

            rendering::opengl_framebuffer_t& light_map = *render_context.light_map_framebuffer;

            if (!light_map.resize(render_context.target_size))
            {
                return;
            }

            rendering::opengl_render_pass_t::begin(light_map, render_context.ambient_light_color);
            render_context.point_light_renderer->begin(render_context.sprite_renderer->get_projection());

            for (usize index = 0; index < system.entities.size(); index++)
            {
                const ecs::entity_t entity = system.entities[index];

                const components::transform_2D_component_t transform = scene_context.scene.get_world_transform_2D(entity);
                const components::point_light_2D_component_t* light   = scene_context.scene.get_point_light_2D(entity);

                rendering::point_light_t point_light;
                point_light.position  = transform.position;
                point_light.radius    = light->radius;
                point_light.color     = light->color;
                point_light.intensity = light->intensity;

                render_context.point_light_renderer->draw(point_light);
            }

            bind_render_target(render_context);

            render_context.sprite_renderer->resume();
            render_context.sprite_renderer->set_material_frame(static_cast<f32>(render_context.elapsed_time), &light_map.get_color_texture());
        }

        void run_camera_2D(ecs::registry_t&, ecs::system_t& system, const f64)
        {
            scene_context_t& scene_context        = get_scene_context(system);
            scene_render_context_t& render_context = scene_context.render_context;

            if (!render_context.is_ready())
            {
                return;
            }

            for (usize index = 0; index < system.entities.size(); index++)
            {
                const ecs::entity_t entity                      = system.entities[index];
                const components::camera_2D_component_t* camera = scene_context.scene.get_camera_2D(entity);

                if (camera->is_primary)
                {
                    render_context.sprite_renderer->begin(render_context.target_size, scene_context.scene.get_world_transform_2D(entity).position, camera->zoom, true);
                    return;
                }
            }
        }

        void run_sprite_2D(ecs::registry_t&, ecs::system_t& system, const f64)
        {
            scene_context_t& scene_context        = get_scene_context(system);
            scene_render_context_t& render_context = scene_context.render_context;

            if (!render_context.is_ready())
            {
                return;
            }

            for (usize index = 0; index < system.entities.size(); index++)
            {
                const ecs::entity_t entity = system.entities[index];

                const components::transform_2D_component_t transform = scene_context.scene.get_world_transform_2D(entity);
                const components::sprite_2D_component_t* sprite       = scene_context.scene.get_sprite_2D(entity);

                const math::vec2_t size(sprite->size.x * transform.scale.x, sprite->size.y * transform.scale.y);

                const rendering::opengl_texture_t* texture = render_context.asset_manager->texture_cache.get(sprite->texture_path);
                const rendering::e_material_shading_models shading_model = sprite->is_lit ? rendering::e_material_shading_models::Lit : rendering::e_material_shading_models::Unlit;

                rendering::material_t* material = render_context.asset_manager->material_cache.get(sprite->material_path, shading_model);

                if (material == nullptr && sprite->is_lit)
                {
                    material = render_context.sprite_renderer->get_default_lit_material();
                }

                if (material != nullptr)
                {
                    render_context.sprite_renderer->draw_with_material(*material, texture, transform.position, size, transform.rotation, sprite->tint);
                    continue;
                }

                render_context.sprite_renderer->draw(texture, transform.position, size, transform.rotation, sprite->tint);
            }
        }

        void run_label_2D(ecs::registry_t&, ecs::system_t& system, const f64)
        {
            scene_context_t& scene_context        = get_scene_context(system);
            scene_render_context_t& render_context = scene_context.render_context;

            if (!render_context.is_ready())
            {
                return;
            }

            for (usize index = 0; index < system.entities.size(); index++)
            {
                const ecs::entity_t entity = system.entities[index];

                const components::transform_2D_component_t transform = scene_context.scene.get_world_transform_2D(entity);
                const components::label_2D_component_t* label         = scene_context.scene.get_label_2D(entity);
                const rendering::font_t* font                         = render_context.asset_manager->font_cache.get(label->font_path, label->font_size);

                if (font != nullptr)
                {
                    render_context.sprite_renderer->draw_text(*font, label->text, transform.position, label->color);
                }
            }
        }
    }

    bool add_default_2D_systems(scene_context_t& scene_context)
    {
        using orders_t = scene_default_2D_system_orders_t;

        const scene_t& scene = scene_context.scene;
        void* data           = &scene_context;

        const std::string transform_2D    = scene.transform_2D_type->name;
        const std::string sprite_2D       = scene.sprite_2D_type->name;
        const std::string script          = scene.script_type->name;
        const std::string velocity_2D     = scene.velocity_2D_type->name;
        const std::string box_collider_2D = scene.box_collider_2D_type->name;
        const std::string audio_source    = scene.audio_source_type->name;
        const std::string label_2D        = scene.label_2D_type->name;
        const std::string camera_2D       = scene.camera_2D_type->name;
        const std::string point_light_2D  = scene.point_light_2D_type->name;

        const ecs::system_descriptor_t descriptors[] =
        {
            ecs::system_descriptor_t(audio_start_system_name, ecs::e_system_stages::Start, orders_t::audio_start, { audio_source }, run_audio_start, data),
            ecs::system_descriptor_t(script_start_system_name, ecs::e_system_stages::Start, orders_t::script_start, { script }, run_script_start, data),

            ecs::system_descriptor_t(physics_2D_system_name, ecs::e_system_stages::Update, orders_t::physics_2D, { transform_2D, velocity_2D }, run_physics_2D, data),
            ecs::system_descriptor_t(collision_2D_system_name, ecs::e_system_stages::Update, orders_t::collision_2D, { transform_2D, box_collider_2D }, run_collision_2D, data),
            ecs::system_descriptor_t(script_system_name, ecs::e_system_stages::Update, orders_t::script, { script }, run_script, data),

            ecs::system_descriptor_t(render_2D_begin_name, ecs::e_system_stages::Render, orders_t::render_2D_begin, { transform_2D }, run_render_2D_begin, data),
            ecs::system_descriptor_t(camera_2D_system_name, ecs::e_system_stages::Render, orders_t::camera_2D, { transform_2D, camera_2D }, run_camera_2D, data),
            ecs::system_descriptor_t(light_2D_system_name, ecs::e_system_stages::Render, orders_t::light_2D, { transform_2D, point_light_2D }, run_light_2D, data),
            ecs::system_descriptor_t(sprite_2D_system_name, ecs::e_system_stages::Render, orders_t::sprite_2D, { transform_2D, sprite_2D }, run_sprite_2D, data),
            ecs::system_descriptor_t(label_2D_system_name, ecs::e_system_stages::Render, orders_t::label_2D, { transform_2D, label_2D }, run_label_2D, data),

            ecs::system_descriptor_t(script_stop_system_name, ecs::e_system_stages::Stop, orders_t::script_stop, { script }, run_script_stop, data),
        };

        bool added_all = true;

        for (const ecs::system_descriptor_t& descriptor : descriptors)
        {
            if (!scene_context.has_system(descriptor.name.c_str()))
            {
                added_all = scene_context.add_system(descriptor) && added_all;
            }
        }

        return added_all;
    }

    void remove_default_2D_systems(scene_context_t& scene_context)
    {
        for (const char* system_name : default_2D_system_names)
        {
            scene_context.remove_system(system_name);
        }
    }

    bool has_any_default_2D_system(const scene_context_t& scene_context)
    {
        for (const char* system_name : default_2D_system_names)
        {
            if (scene_context.has_system(system_name))
            {
                return true;
            }
        }

        return false;
    }
}

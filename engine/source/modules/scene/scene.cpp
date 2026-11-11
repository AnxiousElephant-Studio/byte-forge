#include <byte-forge-engine/modules/scene/scene.h>
#include <byte-forge-engine/modules/math/vectors/vector2.h>
#include <byte-forge-engine/modules/math/vectors/vector4.h>

#include <cstdio>

namespace byteforge::scene
{
    namespace
    {
        const char* const name_type_name            = "name";
        const char* const transform_2D_type_name    = "transform_2D";
        const char* const sprite_2D_type_name       = "sprite_2D";
        const char* const script_type_name          = "script";
        const char* const velocity_2D_type_name     = "velocity_2D";
        const char* const box_collider_2D_type_name = "box_collider_2D";
        const char* const audio_source_type_name    = "audio_source";
        const char* const label_2D_type_name        = "label_2D";
        const char* const camera_2D_type_name       = "camera_2D";
        const char* const point_light_2D_type_name  = "point_light_2D";
        const char* const tags_type_name            = "tags";
        const char* const parent_type_name          = "parent";

        const char* const name_format = "%s";
        const char* const label_text  = "Label";

        const math::vec2_t default_scale(1.0f, 1.0f);
        const math::vec2_t default_sprite_size(100.0f, 100.0f);
        const math::vec4_t default_color(1.0f, 1.0f, 1.0f, 1.0f);
        const f32 default_speed       = 100.0f;
        const f32 default_volume      = 1.0f;
        const f32 default_font_size   = 32.0f;
        const f32 default_camera_zoom = 1.0f;
        const f32 default_light_radius    = 250.0f;
        const f32 default_light_intensity = 1.0f;
        const math::vec4_t default_light_color(1.0f, 0.85f, 0.6f, 1.0f);
    }

    scene_t::scene_t()
        : registry()
        , name_type(nullptr)
        , transform_2D_type(nullptr)
        , sprite_2D_type(nullptr)
        , script_type(nullptr)
        , velocity_2D_type(nullptr)
        , box_collider_2D_type(nullptr)
        , audio_source_type(nullptr)
        , label_2D_type(nullptr)
        , camera_2D_type(nullptr)
        , point_light_2D_type(nullptr)
        , tags_type(nullptr)
        , parent_type(nullptr)
        , entities_to_destroy()
    {
    }

    bool scene_t::initialize()
    {
        name_type            = registry.register_component_type(sizeof(components::name_component_t), name_type_name, nullptr);
        transform_2D_type    = registry.register_component_type(sizeof(components::transform_2D_component_t), transform_2D_type_name, nullptr);
        sprite_2D_type       = registry.register_component_type(sizeof(components::sprite_2D_component_t), sprite_2D_type_name, nullptr);
        script_type          = registry.register_component_type(sizeof(components::script_component_t), script_type_name, nullptr);
        velocity_2D_type     = registry.register_component_type(sizeof(components::velocity_2D_component_t), velocity_2D_type_name, nullptr);
        box_collider_2D_type = registry.register_component_type(sizeof(components::box_collider_2D_component_t), box_collider_2D_type_name, nullptr);
        audio_source_type    = registry.register_component_type(sizeof(components::audio_source_component_t), audio_source_type_name, nullptr);
        label_2D_type        = registry.register_component_type(sizeof(components::label_2D_component_t), label_2D_type_name, nullptr);
        camera_2D_type       = registry.register_component_type(sizeof(components::camera_2D_component_t), camera_2D_type_name, nullptr);
        point_light_2D_type  = registry.register_component_type(sizeof(components::point_light_2D_component_t), point_light_2D_type_name, nullptr);
        tags_type            = registry.register_component_type(sizeof(components::tags_component_t), tags_type_name, nullptr);
        parent_type          = registry.register_component_type(sizeof(components::parent_component_t), parent_type_name, nullptr);

        if (name_type == nullptr || transform_2D_type == nullptr || sprite_2D_type == nullptr || script_type == nullptr ||
            velocity_2D_type == nullptr || box_collider_2D_type == nullptr || audio_source_type == nullptr ||
            label_2D_type == nullptr || camera_2D_type == nullptr || point_light_2D_type == nullptr || tags_type == nullptr || parent_type == nullptr)
        {
            return false;
        }

        return true;
    }

    void scene_t::dispose()
    {
        registry.dispose();

        name_type            = nullptr;
        transform_2D_type    = nullptr;
        sprite_2D_type       = nullptr;
        script_type          = nullptr;
        velocity_2D_type     = nullptr;
        box_collider_2D_type = nullptr;
        audio_source_type    = nullptr;
        label_2D_type        = nullptr;
        camera_2D_type       = nullptr;
        point_light_2D_type  = nullptr;
        tags_type            = nullptr;
        parent_type          = nullptr;
        entities_to_destroy.clear();
    }

    ecs::entity_t scene_t::create_entity(const char* name)
    {
        const ecs::entity_t entity = create_empty_entity(name);

        add_transform_2D(entity);

        return entity;
    }

    ecs::entity_t scene_t::create_empty_entity(const char* name)
    {
        const ecs::entity_t entity = registry.create_entity();

        components::name_component_t name_component = {};
        std::snprintf(name_component.name, components::NAME_MAX_LENGTH, name_format, name);

        registry.add_component_to_entity(*name_type, &name_component, entity);
        registry.spawn_entity(entity);

        return entity;
    }

    components::name_component_t* scene_t::get_name(const ecs::entity_t entity)
    {
        return static_cast<components::name_component_t*>(name_type->get(entity));
    }

    components::transform_2D_component_t* scene_t::get_transform_2D(const ecs::entity_t entity)
    {
        return static_cast<components::transform_2D_component_t*>(transform_2D_type->get(entity));
    }

    components::sprite_2D_component_t* scene_t::get_sprite_2D(const ecs::entity_t entity)
    {
        return static_cast<components::sprite_2D_component_t*>(sprite_2D_type->get(entity));
    }

    components::script_component_t* scene_t::get_script(const ecs::entity_t entity)
    {
        return static_cast<components::script_component_t*>(script_type->get(entity));
    }

    components::velocity_2D_component_t* scene_t::get_velocity_2D(const ecs::entity_t entity)
    {
        return static_cast<components::velocity_2D_component_t*>(velocity_2D_type->get(entity));
    }

    components::box_collider_2D_component_t* scene_t::get_box_collider_2D(const ecs::entity_t entity)
    {
        return static_cast<components::box_collider_2D_component_t*>(box_collider_2D_type->get(entity));
    }

    components::audio_source_component_t* scene_t::get_audio_source(const ecs::entity_t entity)
    {
        return static_cast<components::audio_source_component_t*>(audio_source_type->get(entity));
    }

    components::label_2D_component_t* scene_t::get_label_2D(const ecs::entity_t entity)
    {
        return static_cast<components::label_2D_component_t*>(label_2D_type->get(entity));
    }

    components::camera_2D_component_t* scene_t::get_camera_2D(const ecs::entity_t entity)
    {
        return static_cast<components::camera_2D_component_t*>(camera_2D_type->get(entity));
    }

    void scene_t::add_transform_2D(const ecs::entity_t entity)
    {
        components::transform_2D_component_t transform = {};
        transform.scale = default_scale;

        add_component(*transform_2D_type, &transform, entity);
    }

    void scene_t::add_sprite_2D(const ecs::entity_t entity)
    {
        components::sprite_2D_component_t sprite = {};
        sprite.size = default_sprite_size;
        sprite.tint = default_color;

        add_component(*sprite_2D_type, &sprite, entity);
    }

    void scene_t::add_script(const ecs::entity_t entity)
    {
        components::script_component_t script = {};

        add_component(*script_type, &script, entity);
    }

    void scene_t::add_velocity_2D(const ecs::entity_t entity)
    {
        components::velocity_2D_component_t velocity = {};
        velocity.speed = default_speed;

        add_component(*velocity_2D_type, &velocity, entity);
    }

    void scene_t::add_box_collider_2D(const ecs::entity_t entity)
    {
        components::box_collider_2D_component_t collider = {};
        collider.size = default_sprite_size;

        add_component(*box_collider_2D_type, &collider, entity);
    }

    void scene_t::add_audio_source(const ecs::entity_t entity)
    {
        components::audio_source_component_t audio_source = {};
        audio_source.volume = default_volume;

        add_component(*audio_source_type, &audio_source, entity);
    }

    void scene_t::add_label_2D(const ecs::entity_t entity)
    {
        components::label_2D_component_t label = {};
        std::snprintf(label.text, components::LABEL_TEXT_MAX_LENGTH, name_format, label_text);
        label.font_size = default_font_size;
        label.color     = default_color;

        add_component(*label_2D_type, &label, entity);
    }

    void scene_t::add_camera_2D(const ecs::entity_t entity)
    {
        components::camera_2D_component_t camera = {};
        camera.zoom       = default_camera_zoom;
        camera.is_primary = true;

        add_component(*camera_2D_type, &camera, entity);
    }

    void scene_t::add_point_light_2D(const ecs::entity_t entity)
    {
        components::point_light_2D_component_t light = {};
        light.color     = default_light_color;
        light.radius    = default_light_radius;
        light.intensity = default_light_intensity;

        add_component(*point_light_2D_type, &light, entity);
    }

    components::tags_component_t* scene_t::get_tags(const ecs::entity_t entity)
    {
        return static_cast<components::tags_component_t*>(tags_type->get(entity));
    }

    void scene_t::add_tags(const ecs::entity_t entity)
    {
        components::tags_component_t tags = {};

        add_component(*tags_type, &tags, entity);
    }

    components::point_light_2D_component_t* scene_t::get_point_light_2D(const ecs::entity_t entity)
    {
        return static_cast<components::point_light_2D_component_t*>(point_light_2D_type->get(entity));
    }

    void scene_t::remove_component(ecs::component_type_t& component_type, const ecs::entity_t entity)
    {
        registry.remove_component_from_entity(component_type, entity);
        registry.refresh_entity_systems(entity);
    }

    void scene_t::add_component(ecs::component_type_t& component_type, const void* const component, const ecs::entity_t entity)
    {
        registry.add_component_to_entity(component_type, component, entity);
        registry.refresh_entity_systems(entity);
    }
}

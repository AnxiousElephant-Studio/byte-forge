#pragma once

#include <byte-forge-engine/modules/scene/scene_api.h>
#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/ecs/registry.h>
#include <byte-forge-engine/modules/ecs/entity.h>
#include <byte-forge-engine/modules/components/name_component.h>
#include <byte-forge-engine/modules/components/transform_2D_component.h>
#include <byte-forge-engine/modules/components/sprite_2D_component.h>
#include <byte-forge-engine/modules/components/script_component.h>
#include <byte-forge-engine/modules/components/velocity_2D_component.h>
#include <byte-forge-engine/modules/components/box_collider_2D_component.h>
#include <byte-forge-engine/modules/components/audio_source_component.h>
#include <byte-forge-engine/modules/components/label_2D_component.h>
#include <byte-forge-engine/modules/components/camera_2D_component.h>
#include <byte-forge-engine/modules/components/point_light_2D_component.h>
#include <byte-forge-engine/modules/components/tags_component.h>
#include <byte-forge-engine/modules/components/parent_component.h>
#include <byte-forge-engine/modules/math/vectors/vector2i.h>
#include <byte-forge-engine/modules/rendering/sprite/sprite_renderer.h>
#include <byte-forge-engine/modules/assets/asset_manager.h>

#include <vector>

namespace byteforge::scene
{
    class BF_SCENE_API scene_t
    {
    public:
        ecs::registry_t registry;

        ecs::component_type_t* name_type;
        ecs::component_type_t* transform_2D_type;
        ecs::component_type_t* sprite_2D_type;
        ecs::component_type_t* script_type;
        ecs::component_type_t* velocity_2D_type;
        ecs::component_type_t* box_collider_2D_type;
        ecs::component_type_t* audio_source_type;
        ecs::component_type_t* label_2D_type;
        ecs::component_type_t* camera_2D_type;
        ecs::component_type_t* point_light_2D_type;
        ecs::component_type_t* tags_type;
        ecs::component_type_t* parent_type;

        scene_t();

        bool initialize();
        void dispose();

        ecs::entity_t create_entity(const char* name);
        ecs::entity_t create_empty_entity(const char* name);
        void destroy_entity(const ecs::entity_t entity);
        void queue_destroy(const ecs::entity_t entity);
        std::vector<ecs::entity_t> destroy_queued_entities();
        bool exists(const ecs::entity_t entity) const;

        bool set_parent(const ecs::entity_t child, const ecs::entity_t parent);
        bool attach_to_parent(const ecs::entity_t child, const ecs::entity_t parent);
        void clear_parent(const ecs::entity_t child);
        bool has_parent(const ecs::entity_t entity) const;
        ecs::entity_t get_parent(const ecs::entity_t entity);
        bool is_ancestor(const ecs::entity_t ancestor, const ecs::entity_t entity);
        void get_children(const ecs::entity_t entity, std::vector<ecs::entity_t>& out_children);
        void get_subtree(const ecs::entity_t root, std::vector<ecs::entity_t>& out_entities);
        components::transform_2D_component_t get_world_transform_2D(const ecs::entity_t entity);

        bool find_by_name(const char* name, ecs::entity_t& out_entity);
        void find_with_tag(const char* tag, std::vector<ecs::entity_t>& out_entities);

        components::name_component_t* get_name(const ecs::entity_t entity);
        components::transform_2D_component_t* get_transform_2D(const ecs::entity_t entity);
        components::sprite_2D_component_t* get_sprite_2D(const ecs::entity_t entity);
        components::script_component_t* get_script(const ecs::entity_t entity);
        components::velocity_2D_component_t* get_velocity_2D(const ecs::entity_t entity);
        components::box_collider_2D_component_t* get_box_collider_2D(const ecs::entity_t entity);
        components::audio_source_component_t* get_audio_source(const ecs::entity_t entity);
        components::label_2D_component_t* get_label_2D(const ecs::entity_t entity);
        components::camera_2D_component_t* get_camera_2D(const ecs::entity_t entity);
        components::point_light_2D_component_t* get_point_light_2D(const ecs::entity_t entity);
        components::tags_component_t* get_tags(const ecs::entity_t entity);

        void add_transform_2D(const ecs::entity_t entity);
        void add_sprite_2D(const ecs::entity_t entity);
        void add_script(const ecs::entity_t entity);
        void add_velocity_2D(const ecs::entity_t entity);
        void add_box_collider_2D(const ecs::entity_t entity);
        void add_audio_source(const ecs::entity_t entity);
        void add_label_2D(const ecs::entity_t entity);
        void add_camera_2D(const ecs::entity_t entity);
        void add_point_light_2D(const ecs::entity_t entity);
        void add_tags(const ecs::entity_t entity);
        void remove_component(ecs::component_type_t& component_type, const ecs::entity_t entity);

    private:
        void add_component(ecs::component_type_t& component_type, const void* const component, const ecs::entity_t entity);
        components::transform_2D_component_t get_world_transform_2D(const ecs::entity_t entity, const u32 depth);

        std::vector<ecs::entity_t> entities_to_destroy;
    };
}

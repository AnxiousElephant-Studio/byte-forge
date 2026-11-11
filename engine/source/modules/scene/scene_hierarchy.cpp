#include <byte-forge-engine/modules/scene/scene.h>
#include <byte-forge-engine/modules/math/utils/math_converter.h>

#include <cmath>
#include <cstring>

namespace byteforge::scene
{
    namespace
    {
        const u32 max_hierarchy_depth = 64;

        const math::vec2_t identity_scale(1.0f, 1.0f);

        components::transform_2D_component_t make_identity_transform()
        {
            components::transform_2D_component_t transform = {};
            transform.scale = identity_scale;

            return transform;
        }

        math::vec2_t rotate(const math::vec2_t& vector, const f32 degrees)
        {
            const f32 radians = math::math_converter_t::deg_to_rad(degrees);
            const f32 cosine  = std::cos(radians);
            const f32 sine    = std::sin(radians);

            return math::vec2_t(vector.x * cosine - vector.y * sine, vector.x * sine + vector.y * cosine);
        }

        f32 safe_divide(const f32 value, const f32 divisor)
        {
            return divisor != 0.0f ? value / divisor : value;
        }
    }

    bool scene_t::exists(const ecs::entity_t entity) const
    {
        return name_type != nullptr && name_type->has(entity);
    }

    void scene_t::destroy_entity(const ecs::entity_t entity)
    {
        std::vector<ecs::entity_t> subtree;
        get_subtree(entity, subtree);

        for (usize index = subtree.size(); index > 0; index--)
        {
            registry.kill_entity(subtree[index - 1]);
        }
    }

    void scene_t::queue_destroy(const ecs::entity_t entity)
    {
        entities_to_destroy.push_back(entity);
    }

    std::vector<ecs::entity_t> scene_t::destroy_queued_entities()
    {
        std::vector<ecs::entity_t> destroyed;

        for (const ecs::entity_t entity : entities_to_destroy)
        {
            if (exists(entity))
            {
                get_subtree(entity, destroyed);
                destroy_entity(entity);
            }
        }

        entities_to_destroy.clear();

        return destroyed;
    }

    bool scene_t::set_parent(const ecs::entity_t child, const ecs::entity_t parent)
    {
        const components::transform_2D_component_t child_world  = get_world_transform_2D(child);
        const components::transform_2D_component_t parent_world = get_world_transform_2D(parent);

        if (!attach_to_parent(child, parent))
        {
            return false;
        }

        if (components::transform_2D_component_t* local = get_transform_2D(child))
        {
            const math::vec2_t offset = rotate(child_world.position - parent_world.position, -parent_world.rotation);

            local->position = math::vec2_t(safe_divide(offset.x, parent_world.scale.x), safe_divide(offset.y, parent_world.scale.y));
            local->rotation = child_world.rotation - parent_world.rotation;
            local->scale    = math::vec2_t(safe_divide(child_world.scale.x, parent_world.scale.x), safe_divide(child_world.scale.y, parent_world.scale.y));
        }

        return true;
    }

    bool scene_t::attach_to_parent(const ecs::entity_t child, const ecs::entity_t parent)
    {
        if (child == parent || !exists(child) || !exists(parent) || is_ancestor(child, parent))
        {
            return false;
        }

        components::parent_component_t parent_component = {};
        parent_component.parent = parent;

        registry.add_component_to_entity(*parent_type, &parent_component, child);

        return true;
    }

    void scene_t::clear_parent(const ecs::entity_t child)
    {
        if (!has_parent(child))
        {
            return;
        }

        const components::transform_2D_component_t child_world = get_world_transform_2D(child);

        registry.remove_component_from_entity(*parent_type, child);

        if (components::transform_2D_component_t* local = get_transform_2D(child))
        {
            *local = child_world;
        }
    }

    bool scene_t::has_parent(const ecs::entity_t entity) const
    {
        return parent_type != nullptr && parent_type->has(entity);
    }

    ecs::entity_t scene_t::get_parent(const ecs::entity_t entity)
    {
        const components::parent_component_t* parent_component = static_cast<components::parent_component_t*>(parent_type->get(entity));

        return parent_component != nullptr ? parent_component->parent : entity;
    }

    bool scene_t::is_ancestor(const ecs::entity_t ancestor, const ecs::entity_t entity)
    {
        ecs::entity_t current = entity;

        for (u32 depth = 0; depth < max_hierarchy_depth && has_parent(current); depth++)
        {
            current = get_parent(current);

            if (current == ancestor)
            {
                return true;
            }
        }

        return false;
    }

    void scene_t::get_children(const ecs::entity_t entity, std::vector<ecs::entity_t>& out_children)
    {
        for (const ecs::entity_t candidate : registry.entities)
        {
            if (has_parent(candidate) && get_parent(candidate) == entity)
            {
                out_children.push_back(candidate);
            }
        }
    }

    void scene_t::get_subtree(const ecs::entity_t root, std::vector<ecs::entity_t>& out_entities)
    {
        const usize first_index = out_entities.size();
        out_entities.push_back(root);

        for (usize index = first_index; index < out_entities.size(); index++)
        {
            get_children(out_entities[index], out_entities);
        }
    }

    components::transform_2D_component_t scene_t::get_world_transform_2D(const ecs::entity_t entity)
    {
        return get_world_transform_2D(entity, 0);
    }

    components::transform_2D_component_t scene_t::get_world_transform_2D(const ecs::entity_t entity, const u32 depth)
    {
        const components::transform_2D_component_t* local_pointer = get_transform_2D(entity);
        const components::transform_2D_component_t local          = local_pointer != nullptr ? *local_pointer : make_identity_transform();

        if (!has_parent(entity) || depth >= max_hierarchy_depth)
        {
            return local;
        }

        const components::transform_2D_component_t parent = get_world_transform_2D(get_parent(entity), depth + 1);

        components::transform_2D_component_t world = {};
        world.position = parent.position + rotate(math::vec2_t(local.position.x * parent.scale.x, local.position.y * parent.scale.y), parent.rotation);
        world.rotation = parent.rotation + local.rotation;
        world.scale    = math::vec2_t(local.scale.x * parent.scale.x, local.scale.y * parent.scale.y);

        return world;
    }

    bool scene_t::find_by_name(const char* name, ecs::entity_t& out_entity)
    {
        for (const ecs::entity_t entity : registry.entities)
        {
            const components::name_component_t* name_component = get_name(entity);

            if (name_component != nullptr && std::strcmp(name_component->name, name) == 0)
            {
                out_entity = entity;
                return true;
            }
        }

        return false;
    }

    void scene_t::find_with_tag(const char* tag, std::vector<ecs::entity_t>& out_entities)
    {
        for (const ecs::entity_t entity : registry.entities)
        {
            const components::tags_component_t* tags = get_tags(entity);

            if (tags != nullptr && tags->has_tag(tag))
            {
                out_entities.push_back(entity);
            }
        }
    }
}

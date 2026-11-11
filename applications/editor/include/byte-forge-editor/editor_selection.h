#pragma once

#include <byte-forge-engine/modules/ecs/entity.h>

namespace byteforge::editor
{
    enum class e_gizmo_targets
    {
        Transform2D,
        BoxCollider2D
    };

    struct editor_selection_t
    {
    public:
        ecs::entity_t entity;
        bool has_entity;
        e_gizmo_targets gizmo_target;

        editor_selection_t() : entity(), has_entity(false), gizmo_target(e_gizmo_targets::Transform2D) {}

        void select(const ecs::entity_t selected_entity)
        {
            if (!has_entity || !(entity == selected_entity))
            {
                gizmo_target = e_gizmo_targets::Transform2D;
            }

            entity     = selected_entity;
            has_entity = true;
        }

        void clear()
        {
            has_entity   = false;
            gizmo_target = e_gizmo_targets::Transform2D;
        }

        bool is_selected(const ecs::entity_t candidate_entity) const
        {
            return has_entity && entity == candidate_entity;
        }
    };
}

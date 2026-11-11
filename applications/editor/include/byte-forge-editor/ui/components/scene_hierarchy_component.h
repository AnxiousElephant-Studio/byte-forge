#pragma once

#include <byte-forge-editor/editor_application_context.h>

#include <byte-forge-engine/modules/ecs/entity.h>

#include <string>

namespace byteforge::editor
{
    enum class e_pending_hierarchy_changes
    {
        None,
        Reparent,
        Unparent,
        Delete
    };

    struct pending_hierarchy_change_t
    {
    public:
        e_pending_hierarchy_changes type = e_pending_hierarchy_changes::None;
        ecs::entity_t entity;
        ecs::entity_t target;
    };

    class scene_hierarchy_component_t
    {
    public:
        void draw(editor_application_context_t& context);

    private:
        void draw_level_bar(editor_application_context_t& context);
        void draw_new_level_popup(editor_application_context_t& context);
        void draw_unsaved_changes_popup(editor_application_context_t& context);

        void draw_create_menu(editor_application_context_t& context, const ecs::entity_t* parent);
        void draw_entities(editor_application_context_t& context);
        void draw_entity_node(editor_application_context_t& context, const ecs::entity_t entity);
        void draw_entity_context_menu(editor_application_context_t& context, const ecs::entity_t entity);
        void accept_hierarchy_drop(editor_application_context_t& context, const ecs::entity_t* target);
        void apply_pending_hierarchy_change(editor_application_context_t& context);

        void request_level_change(editor_application_context_t& context, const std::string& level_path, const std::string& level_name);
        void apply_pending_level_change(editor_application_context_t& context);

        pending_hierarchy_change_t pending_change;
        std::string pending_level_path;
        std::string pending_level_name;
        std::string new_level_error;
        char new_level_name_buffer[64] = {};
        bool should_open_unsaved_changes_popup = false;
    };
}

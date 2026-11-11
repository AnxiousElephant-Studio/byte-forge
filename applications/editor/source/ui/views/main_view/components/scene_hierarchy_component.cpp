#include <byte-forge-editor/ui/components/scene_hierarchy_component.h>
#include <byte-forge-editor/ui/utils/editor_colors.h>
#include <byte-forge-editor/ui/utils/drag_drop.h>
#include <byte-forge-editor/ui/utils/file_name_validation.h>
#include <byte-forge-editor/ui/utils/scoped_window.h>

#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/components/name_component.h>
#include <byte-forge-engine/modules/io/file_system.h>
#include <byte-forge-engine/modules/level/level_file.h>
#include <byte-forge-engine/modules/level/stamp_file.h>

#include <vector>

#include <imgui.h>

namespace byteforge::editor
{
    namespace
    {
        const char* const window_title            = "Scene Hierarchy";
        const char* const add_entity_label        = "+";
        const char* const add_entity_popup_id     = "add_entity_popup";
        const char* const window_context_popup_id = "hierarchy_window_context";
        const char* const create_child_label      = "Create Child";
        const char* const unparent_label          = "Unparent";
        const char* const save_stamp_label       = "Save as Stamp";
        const char* const entity_node_id          = "entity_node";
        const char* const entity_payload_type     = "BF_ENTITY";
        const char* const create_entity_label     = "Create Entity";
        const char* const create_empty_label      = "Create Empty";
        const char* const create_light_label      = "Create Light";
        const char* const default_light_name      = "Point Light";
        const char* const delete_entity_label     = "Delete";
        const char* const default_entity_name     = "Entity";
        const char* const unnamed_entity_label    = "<unnamed>";

        const char* const level_combo_id       = "##current_level";
        const char* const no_level_label       = "No level";
        const char* const dirty_marker         = "*";
        const char* const new_level_label      = "New";
        const char* const save_level_label     = "Save";
        const char* const start_level_label    = "Set as start level";
        const char* const new_level_popup_id   = "New Level";
        const char* const new_level_name_label = "Name";
        const char* const create_label         = "Create";
        const char* const cancel_label         = "Cancel";
        const char* const unsaved_popup_id     = "Unsaved Changes";
        const char* const unsaved_message      = "Save the changes to '%s' before switching levels?";
        const char* const dont_save_label      = "Don't Save";
        const char* const level_root_id        = "level_root";
        const char* const level_root_format    = "%s%s";

        const char* const missing_name_error = "Enter a level name.";
        const char* const invalid_name_error = "The name contains characters that are not allowed in a file name.";
        const char* const non_ascii_error    = "Level names with accented characters are not supported yet.";

        const float new_button_spacing = 8.0f;
    }

    void scene_hierarchy_component_t::draw_level_bar(editor_application_context_t& context)
    {
        const std::string preview = context.current_level_path.empty()
            ? std::string(no_level_label)
            : context.current_level_name + (context.is_level_dirty ? dirty_marker : "");

        const float new_button_width = ImGui::CalcTextSize(new_level_label).x + ImGui::GetStyle().FramePadding.x * 2.0f;

        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - new_button_width - new_button_spacing);

        if (ImGui::BeginCombo(level_combo_id, preview.c_str()))
        {
            std::vector<io::directory_entry_t> entries;
            io::file_system_t::list_directory(context.current_project.settings.levels_directory.c_str(), entries);

            for (const io::directory_entry_t& entry : entries)
            {
                if (entry.is_directory || entry.extension != level::level_file_t::extension)
                {
                    continue;
                }

                const bool is_current = entry.path == context.current_level_path;

                if (ImGui::Selectable(entry.name.c_str(), is_current) && !is_current)
                {
                    request_level_change(context, entry.path, std::string());
                }
            }

            ImGui::EndCombo();
        }

        ImGui::SameLine(0.0f, new_button_spacing);

        if (ImGui::Button(new_level_label))
        {
            new_level_name_buffer[0] = '\0';
            new_level_error.clear();
            ImGui::OpenPopup(new_level_popup_id);
        }

        draw_new_level_popup(context);

        if (should_open_unsaved_changes_popup)
        {
            should_open_unsaved_changes_popup = false;
            ImGui::OpenPopup(unsaved_popup_id);
        }

        draw_unsaved_changes_popup(context);

        if (!context.requests.level_error.empty())
        {
            ImGui::PushTextWrapPos(0.0f);
            ImGui::TextColored(editor_colors_t::failure, "%s", context.requests.level_error.c_str());
            ImGui::PopTextWrapPos();
        }
    }

    void scene_hierarchy_component_t::draw_new_level_popup(editor_application_context_t& context)
    {
        if (!ImGui::BeginPopup(new_level_popup_id))
        {
            return;
        }

        ImGui::InputText(new_level_name_label, new_level_name_buffer, sizeof(new_level_name_buffer));

        if (!new_level_error.empty())
        {
            ImGui::TextColored(editor_colors_t::failure, "%s", new_level_error.c_str());
        }

        if (ImGui::Button(create_label))
        {
            if (new_level_name_buffer[0] == '\0')
            {
                new_level_error = missing_name_error;
            }
            else if (has_invalid_file_name_characters(new_level_name_buffer))
            {
                new_level_error = invalid_name_error;
            }
            else if (!is_ascii_text(new_level_name_buffer))
            {
                new_level_error = non_ascii_error;
            }
            else
            {
                request_level_change(context, std::string(), new_level_name_buffer);
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::SameLine();

        if (ImGui::Button(cancel_label))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    void scene_hierarchy_component_t::draw_unsaved_changes_popup(editor_application_context_t& context)
    {
        if (!ImGui::BeginPopupModal(unsaved_popup_id, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            return;
        }

        ImGui::Text(unsaved_message, context.current_level_name.c_str());
        ImGui::Spacing();

        if (ImGui::Button(save_level_label))
        {
            context.requests.is_level_save_requested = true;
            apply_pending_level_change(context);
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button(dont_save_label))
        {
            apply_pending_level_change(context);
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button(cancel_label))
        {
            pending_level_path.clear();
            pending_level_name.clear();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    void scene_hierarchy_component_t::draw(editor_application_context_t& context)
    {
        const scoped_window_t window(window_title);

        if (!window.is_visible())
        {
            return;
        }

        draw_level_bar(context);

        if (ImGui::Button(add_entity_label))
        {
            ImGui::OpenPopup(add_entity_popup_id);
        }

        if (ImGui::BeginPopup(add_entity_popup_id))
        {
            draw_create_menu(context, nullptr);
            ImGui::EndPopup();
        }

        if (!context.requests.stamp_error.empty())
        {
            ImGui::PushTextWrapPos(0.0f);
            ImGui::TextColored(editor_colors_t::failure, "%s", context.requests.stamp_error.c_str());
            ImGui::PopTextWrapPos();
        }

        ImGui::Separator();

        draw_entities(context);

        if (ImGui::BeginPopupContextWindow(window_context_popup_id, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
        {
            draw_create_menu(context, nullptr);
            ImGui::EndPopup();
        }

        apply_pending_hierarchy_change(context);
    }

    void scene_hierarchy_component_t::draw_create_menu(editor_application_context_t& context, const ecs::entity_t* parent)
    {
        scene::scene_t& scene = context.scene_context.scene;
        ecs::entity_t created;
        bool was_created = false;

        if (ImGui::MenuItem(create_entity_label))
        {
            created     = scene.create_entity(default_entity_name);
            was_created = true;
        }

        if (ImGui::MenuItem(create_empty_label))
        {
            created     = scene.create_empty_entity(default_entity_name);
            was_created = true;
        }

        if (ImGui::MenuItem(create_light_label))
        {
            created = scene.create_entity(default_light_name);
            scene.add_point_light_2D(created);
            was_created = true;
        }

        if (!was_created)
        {
            return;
        }

        if (parent != nullptr)
        {
            scene.attach_to_parent(created, *parent);
        }

        context.selection.select(created);
    }

    void scene_hierarchy_component_t::draw_entities(editor_application_context_t& context)
    {
        scene::scene_t& scene = context.scene_context.scene;

        const char* const root_name = context.current_level_path.empty() ? no_level_label : context.current_level_name.c_str();
        const char* const root_mark = context.is_level_dirty ? dirty_marker : "";

        const bool is_root_open = ImGui::TreeNodeEx(level_root_id, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth, level_root_format, root_name, root_mark);

        accept_hierarchy_drop(context, nullptr);

        if (ImGui::BeginPopupContextItem())
        {
            const bool has_level = !context.current_level_path.empty();

            if (ImGui::MenuItem(save_level_label, nullptr, false, has_level))
            {
                context.requests.is_level_save_requested = true;
            }

            if (ImGui::MenuItem(start_level_label, nullptr, context.current_level_path == context.current_project.start_level, has_level))
            {
                context.requests.is_start_level_change_requested = true;
            }

            ImGui::EndPopup();
        }

        if (!is_root_open)
        {
            return;
        }

        const std::vector<ecs::entity_t> entities = scene.registry.entities;

        for (const ecs::entity_t entity : entities)
        {
            if (!scene.has_parent(entity))
            {
                draw_entity_node(context, entity);
            }
        }

        ImGui::TreePop();
    }

    void scene_hierarchy_component_t::draw_entity_node(editor_application_context_t& context, const ecs::entity_t entity)
    {
        scene::scene_t& scene = context.scene_context.scene;

        std::vector<ecs::entity_t> children;
        scene.get_children(entity, children);

        const components::name_component_t* name = scene.get_name(entity);

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;

        if (children.empty())
        {
            flags |= ImGuiTreeNodeFlags_Leaf;
        }

        if (context.selection.is_selected(entity))
        {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        ImGui::PushID(static_cast<i32>(entity.id));

        const bool is_open = ImGui::TreeNodeEx(entity_node_id, flags, "%s", name != nullptr ? name->name : unnamed_entity_label);

        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
        {
            context.selection.select(entity);
        }

        if (ImGui::BeginDragDropSource())
        {
            ImGui::SetDragDropPayload(entity_payload_type, &entity, sizeof(entity));
            ImGui::TextUnformatted(name != nullptr ? name->name : unnamed_entity_label);
            ImGui::EndDragDropSource();
        }

        accept_hierarchy_drop(context, &entity);
        draw_entity_context_menu(context, entity);

        if (is_open)
        {
            for (const ecs::entity_t child : children)
            {
                draw_entity_node(context, child);
            }

            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    void scene_hierarchy_component_t::draw_entity_context_menu(editor_application_context_t& context, const ecs::entity_t entity)
    {
        if (!ImGui::BeginPopupContextItem())
        {
            return;
        }

        scene::scene_t& scene = context.scene_context.scene;

        if (ImGui::BeginMenu(create_child_label))
        {
            draw_create_menu(context, &entity);
            ImGui::EndMenu();
        }

        if (scene.has_parent(entity) && ImGui::MenuItem(unparent_label))
        {
            pending_change = { e_pending_hierarchy_changes::Unparent, entity, entity };
        }

        if (ImGui::MenuItem(save_stamp_label))
        {
            context.requests.stamp_root_to_save     = entity;
            context.requests.is_stamp_save_requested = true;
        }

        ImGui::Separator();

        if (ImGui::MenuItem(delete_entity_label))
        {
            pending_change = { e_pending_hierarchy_changes::Delete, entity, entity };
        }

        ImGui::EndPopup();
    }

    void scene_hierarchy_component_t::accept_hierarchy_drop(editor_application_context_t& context, const ecs::entity_t* target)
    {
        if (!ImGui::BeginDragDropTarget())
        {
            return;
        }

        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(entity_payload_type))
        {
            const ecs::entity_t dragged = *static_cast<const ecs::entity_t*>(payload->Data);

            if (target != nullptr)
            {
                pending_change = { e_pending_hierarchy_changes::Reparent, dragged, *target };
            }
            else
            {
                pending_change = { e_pending_hierarchy_changes::Unparent, dragged, dragged };
            }
        }

        std::string asset_path;

        if (accept_asset_path_drop(asset_path) && io::file_system_t::get_extension(asset_path.c_str()) == level::stamp_file_t::extension)
        {
            context.requests.stamp_to_instantiate = asset_path;
        }

        ImGui::EndDragDropTarget();
    }

    void scene_hierarchy_component_t::apply_pending_hierarchy_change(editor_application_context_t& context)
    {
        scene::scene_t& scene = context.scene_context.scene;
        const pending_hierarchy_change_t change = pending_change;

        pending_change = pending_hierarchy_change_t();

        switch (change.type)
        {
            case e_pending_hierarchy_changes::None:
                return;

            case e_pending_hierarchy_changes::Reparent:
                scene.set_parent(change.entity, change.target);
                return;

            case e_pending_hierarchy_changes::Unparent:
                scene.clear_parent(change.entity);
                return;

            case e_pending_hierarchy_changes::Delete:
                if (context.selection.is_selected(change.entity) || scene.is_ancestor(change.entity, context.selection.entity))
                {
                    context.selection.clear();
                }

                if (context.scene_context.scene_player.is_running())
                {
                    scene.queue_destroy(change.entity);
                }
                else
                {
                    scene.destroy_entity(change.entity);
                }

                return;
        }
    }

    void scene_hierarchy_component_t::request_level_change(editor_application_context_t& context, const std::string& level_path, const std::string& level_name)
    {
        pending_level_path = level_path;
        pending_level_name = level_name;

        context.requests.level_error.clear();

        if (context.is_level_dirty)
        {
            should_open_unsaved_changes_popup = true;
            return;
        }

        apply_pending_level_change(context);
    }

    void scene_hierarchy_component_t::apply_pending_level_change(editor_application_context_t& context)
    {
        context.requests.level_file_to_open   = pending_level_path;
        context.requests.level_name_to_create = pending_level_name;

        pending_level_path.clear();
        pending_level_name.clear();
    }
}

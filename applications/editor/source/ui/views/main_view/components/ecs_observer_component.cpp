#include <byte-forge-editor/ui/components/ecs_observer_component.h>
#include <byte-forge-editor/ui/utils/editor_colors.h>
#include <byte-forge-editor/ui/utils/scoped_window.h>

#include <byte-forge-engine/modules/components/name_component.h>
#include <byte-forge-engine/modules/ecs/e_system_stages.h>
#include <byte-forge-engine/modules/scene/scene_default_2D_systems.h>

#include <string>

#include <imgui.h>

#include <cstdio>

namespace byteforge::editor
{
    namespace
    {
        const char* const window_title        = "ECS Observer";
        const char* const entities_tab        = "Entities";
        const char* const component_types_tab = "Component Types";
        const char* const systems_tab         = "Systems";
        const char* const validation_tab      = "Validation";

        const char* const entities_table_id = "ecs_entities_table";
        const char* const types_table_id    = "ecs_types_table";
        const char* const systems_table_id  = "ecs_systems_table";

        const char* const id_column              = "ID";
        const char* const name_column            = "Name";
        const char* const components_column      = "Components";
        const char* const bitset_column          = "Bitset";
        const char* const type_id_column         = "Bit";
        const char* const size_column            = "Size";
        const char* const count_column           = "Count";
        const char* const memory_column          = "Memory";
        const char* const required_column        = "Requires";
        const char* const system_entities_column = "Entities";
        const char* const stage_column           = "Stage";
        const char* const order_column           = "Order";
        const char* const enabled_column         = "Enabled";
        const char* const enabled_checkbox_id    = "##system_enabled";
        const char* const order_input_id         = "##system_order";
        const char* const remove_system_label    = "Remove";
        const char* const add_default_2D_label    = "Add default 2D systems";
        const char* const remove_default_2D_label = "Remove default 2D systems";
        const char* const no_systems_label        = "No systems registered. The scene does nothing until systems are added.";

        const f32 order_input_width = 70.0f;

        const char* const unnamed_entity_label = "<unnamed>";
        const char* const no_components_label  = "-";
        const char* const separator            = ", ";

        const char* const validate_label       = "Validate Now";
        const char* const validate_every_label = "Validate Every Frame";
        const char* const not_validated_label  = "Not validated yet.";
        const char* const valid_label          = "No inconsistencies found.";
        const char* const issues_format        = "%zu inconsistencies found:";

        const char* const id_format     = "%llu";
        const char* const bitset_format = "0x%016llX";
        const char* const number_format = "%zu";
        const char* const bytes_format  = "%zu B";
        const char* const text_format   = "%s";
        const char* const join_format   = "%s%s";
        const char* const tab_bar_id    = "ecs_observer_tabs";

        const i32 entities_column_count = 4;
        const i32 types_column_count    = 5;
        const i32 systems_column_count  = 7;

        const usize id_text_buffer_size         = 24;
        const usize components_text_buffer_size = 256;
        const usize required_text_buffer_size   = 256;

        void append_component_names(const ecs::registry_t& registry, const u64 bitset, char* buffer, const usize buffer_size)
        {
            buffer[0] = '\0';
            usize length = 0;

            for (usize bit = 0; bit < registry.component_types.size(); bit++)
            {
                if ((bitset & (1ULL << bit)) == 0)
                {
                    continue;
                }

                const i32 written = std::snprintf(buffer + length, buffer_size - length, join_format, length == 0 ? "" : separator, registry.component_types[bit]->name.c_str());

                if (written < 0 || static_cast<usize>(written) >= buffer_size - length)
                {
                    return;
                }

                length += static_cast<usize>(written);
            }

            if (length == 0)
            {
                std::snprintf(buffer, buffer_size, text_format, no_components_label);
            }
        }
    }

    void ecs_observer_component_t::draw(editor_application_context_t& context)
    {
        const scoped_window_t window(window_title);

        if (!window.is_visible())
        {
            return;
        }

        if (ImGui::BeginTabBar(tab_bar_id))
        {
            if (ImGui::BeginTabItem(entities_tab))
            {
                draw_entities(context);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem(component_types_tab))
            {
                draw_component_types(context);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem(systems_tab))
            {
                draw_systems(context);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem(validation_tab))
            {
                draw_validation(context);
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }

    void ecs_observer_component_t::draw_entities(editor_application_context_t& context)
    {
        const ecs::registry_t& registry = context.scene_context.scene.registry;

        if (!ImGui::BeginTable(entities_table_id, entities_column_count, ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp))
        {
            return;
        }

        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn(id_column, ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn(name_column);
        ImGui::TableSetupColumn(components_column);
        ImGui::TableSetupColumn(bitset_column, ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();

        ImGuiListClipper clipper;
        clipper.Begin(static_cast<i32>(registry.entities.size()));

        while (clipper.Step())
        {
            for (i32 row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
            {
                const ecs::entity_t entity = registry.entities[static_cast<usize>(row)];
                const u64 bitset           = entity.id < registry.entities_components_bitsets.size() ? registry.entities_components_bitsets[entity.id] : 0;

                const components::name_component_t* name = context.scene_context.scene.get_name(entity);

                char id_text[id_text_buffer_size];
                std::snprintf(id_text, sizeof(id_text), id_format, static_cast<unsigned long long>(entity.id));

                char components_text[components_text_buffer_size];
                append_component_names(registry, bitset, components_text, sizeof(components_text));

                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                const bool selected = context.selection.is_selected(entity);

                if (ImGui::Selectable(id_text, selected, ImGuiSelectableFlags_SpanAllColumns))
                {
                    context.selection.select(entity);
                }

                ImGui::TableNextColumn();
                ImGui::TextUnformatted(name != nullptr ? name->name : unnamed_entity_label);
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(components_text);
                ImGui::TableNextColumn();
                ImGui::Text(bitset_format, static_cast<unsigned long long>(bitset));
            }
        }

        ImGui::EndTable();
    }

    void ecs_observer_component_t::draw_component_types(editor_application_context_t& context)
    {
        const ecs::registry_t& registry = context.scene_context.scene.registry;

        if (!ImGui::BeginTable(types_table_id, types_column_count, ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp))
        {
            return;
        }

        ImGui::TableSetupColumn(name_column);
        ImGui::TableSetupColumn(type_id_column, ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn(size_column, ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn(count_column, ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn(memory_column, ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();

        for (usize index = 0; index < registry.component_types.size(); index++)
        {
            const ecs::component_type_t& type = *registry.component_types[index];

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(type.name.c_str());
            ImGui::TableNextColumn();
            ImGui::Text(number_format, static_cast<usize>(type.id));
            ImGui::TableNextColumn();
            ImGui::Text(bytes_format, type.size);
            ImGui::TableNextColumn();
            ImGui::Text(number_format, type.get_count());
            ImGui::TableNextColumn();
            ImGui::Text(bytes_format, type.get_memory_bytes());
        }

        ImGui::EndTable();
    }

    void ecs_observer_component_t::draw_systems(editor_application_context_t& context)
    {
        ecs::registry_t& registry = context.scene_context.scene.registry;

        draw_system_actions(context);

        if (registry.systems.empty())
        {
            ImGui::TextUnformatted(no_systems_label);
            return;
        }

        if (!ImGui::BeginTable(systems_table_id, systems_column_count, ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp))
        {
            return;
        }

        std::string system_to_remove;
        std::string system_to_reorder;
        i32 new_system_order = 0;

        ImGui::TableSetupColumn(enabled_column, ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn(stage_column, ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn(order_column, ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn(name_column);
        ImGui::TableSetupColumn(required_column);
        ImGui::TableSetupColumn(system_entities_column, ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn(remove_system_label, ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();

        for (usize index = 0; index < registry.systems.size(); index++)
        {
            ecs::system_t& system = *registry.systems[index];

            char required_text[required_text_buffer_size];
            append_component_names(registry, system.required_entity_components_bitset, required_text, sizeof(required_text));

            ImGui::PushID(static_cast<i32>(index));

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Checkbox(enabled_checkbox_id, &system.is_enabled);
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(ecs::get_system_stage_name(system.stage));
            ImGui::TableNextColumn();

            i32 edited_order = system.order;
            ImGui::SetNextItemWidth(order_input_width);

            if (ImGui::InputInt(order_input_id, &edited_order, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue) && edited_order != system.order)
            {
                system_to_reorder = system.name;
                new_system_order  = edited_order;
            }
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(system.name.c_str());
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(required_text);
            ImGui::TableNextColumn();
            ImGui::Text(number_format, system.entities.size());
            ImGui::TableNextColumn();

            if (ImGui::SmallButton(remove_system_label))
            {
                system_to_remove = system.name;
            }

            ImGui::PopID();
        }

        ImGui::EndTable();

        if (!system_to_reorder.empty())
        {
            context.scene_context.set_system_order(system_to_reorder.c_str(), new_system_order);
        }

        if (!system_to_remove.empty())
        {
            context.scene_context.remove_system(system_to_remove.c_str());
        }
    }

    void ecs_observer_component_t::draw_system_actions(editor_application_context_t& context)
    {
        scene::scene_context_t& scene_context = context.scene_context;

        if (ImGui::Button(add_default_2D_label))
        {
            scene::add_default_2D_systems(scene_context);
        }

        ImGui::SameLine();
        ImGui::BeginDisabled(!scene::has_any_default_2D_system(scene_context));

        if (ImGui::Button(remove_default_2D_label))
        {
            scene::remove_default_2D_systems(scene_context);
        }

        ImGui::EndDisabled();
        ImGui::Separator();
    }

    void ecs_observer_component_t::draw_validation(editor_application_context_t& context)
    {
        if (ImGui::Button(validate_label) || validate_every_frame)
        {
            context.scene_context.scene.registry.validate(validation_issues);
            has_validated = true;
        }

        ImGui::SameLine();
        ImGui::Checkbox(validate_every_label, &validate_every_frame);

        if (!has_validated)
        {
            ImGui::TextUnformatted(not_validated_label);
            return;
        }

        if (validation_issues.empty())
        {
            ImGui::TextColored(editor_colors_t::success, text_format, valid_label);
            return;
        }

        ImGui::TextColored(editor_colors_t::failure, issues_format, validation_issues.size());

        for (usize index = 0; index < validation_issues.size(); index++)
        {
            ImGui::BulletText(text_format, validation_issues[index].c_str());
        }
    }
}

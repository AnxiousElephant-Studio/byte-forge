#include <byte-forge-editor/ui/components/object_inspector_component.h>
#include <byte-forge-editor/ui/utils/scoped_window.h>
#include <byte-forge-editor/ui/utils/drag_drop.h>
#include <byte-forge-editor/ui/utils/editor_colors.h>

#include <byte-forge-engine/api/types.h>

#include <imgui.h>

#include <cstdio>

namespace byteforge::editor
{
    namespace
    {
        const char* const window_title               = "Object Inspector";
        const char* const no_selection_label         = "No entity selected.";
        const char* const name_label                 = "Name";
        const char* const transform_2D_header        = "Transform 2D";
        const char* const tags_header                = "Tags";
        const char* const tags_label                 = "##tags";
        const char* const tags_hint                  = "player, enemy";
        const char* const world_position_format      = "World: (%.1f, %.1f)";
        const char* const sprite_2D_header           = "Sprite 2D";
        const char* const label_2D_header            = "Label 2D";
        const char* const velocity_2D_header         = "Velocity 2D";
        const char* const box_collider_2D_header     = "Box Collider 2D";
        const char* const camera_2D_header           = "Camera 2D";
        const char* const point_light_2D_header      = "Point Light 2D";
        const char* const audio_source_header        = "Audio Source";
        const char* const script_header              = "Script";
        const char* const position_label             = "Position";
        const char* const rotation_label             = "Rotation";
        const char* const scale_label                = "Scale";
        const char* const texture_label              = "Texture";
        const char* const size_label                 = "Size";
        const char* const tint_label                 = "Tint";
        const char* const text_label                 = "Text";
        const char* const font_label                 = "Font";
        const char* const font_size_label            = "Font Size";
        const char* const color_label                = "Color";
        const char* const velocity_label             = "Velocity";
        const char* const speed_label                = "Speed";
        const char* const offset_label               = "Offset";
        const char* const zoom_label                 = "Zoom";
        const char* const primary_label              = "Primary";
        const char* const clip_label                 = "Clip";
        const char* const volume_label               = "Volume";
        const char* const loop_label                 = "Loop";
        const char* const play_on_start_label        = "Play On Start";
        const char* const script_path_label          = "Path";
        const char* const edit_script_label          = "Edit Script";
        const char* const material_label             = "Material";
        const char* const lit_label                  = "Lit";
        const char* const reload_material_label      = "Reload Material";
        const char* const material_failed_label      = "The material could not be loaded or compiled. See the Console; the sprite uses the default shader until it is fixed.";
        const char* const edit_material_label        = "Edit Material";
        const char* const radius_label               = "Radius";
        const char* const intensity_label            = "Intensity";
        const char* const lit_sprites_hint_label     = "Lights only affect sprites with Lit checked.";
        const char* const lighting_unavailable_label = "Lighting is not available: the point light renderer failed to start.";
        const char* const remove_component_label     = "Remove Component";
        const char* const add_component_label        = "Add Component";
        const char* const add_component_popup        = "add_component_popup";

        const f32 position_drag_speed  = 1.0f;
        const f32 rotation_drag_speed  = 0.5f;
        const f32 scale_drag_speed     = 0.01f;
        const f32 size_drag_speed      = 1.0f;
        const f32 font_size_min        = 4.0f;
        const f32 font_size_max        = 256.0f;
        const f32 zoom_drag_speed      = 0.01f;
        const f32 zoom_min             = 0.01f;
        const f32 zoom_max             = 100.0f;
        const f32 radius_drag_speed    = 1.0f;
        const f32 radius_min           = 1.0f;
        const f32 radius_max           = 10000.0f;
        const f32 intensity_drag_speed = 0.01f;
        const f32 intensity_min        = 0.0f;
        const f32 intensity_max        = 100.0f;
        const f32 volume_min           = 0.0f;
        const f32 volume_max           = 1.0f;

        void path_field(const char* label, char* buffer, usize buffer_size)
        {
            ImGui::InputText(label, buffer, buffer_size);
            accept_asset_path_drop(buffer, buffer_size);
        }
    }

    void object_inspector_component_t::draw(editor_application_context_t& context)
    {
        const scoped_window_t window(window_title);

        if (!window.is_visible())
        {
            return;
        }

        if (context.selection.has_entity && !context.scene_context.scene.exists(context.selection.entity))
        {
            context.selection.clear();
        }

        if (!context.selection.has_entity)
        {
            ImGui::TextUnformatted(no_selection_label);
            return;
        }

        ImGui::PushID(static_cast<i32>(context.selection.entity.id));

        draw_name(context);
        draw_tags(context);
        draw_transform_2D(context);
        draw_sprite_2D(context);
        draw_label_2D(context);
        draw_velocity_2D(context);
        draw_box_collider_2D(context);
        draw_camera_2D(context);
        draw_point_light_2D(context);
        draw_audio_source(context);
        draw_script(context);
        draw_add_component(context);

        ImGui::PopID();
    }

    bool object_inspector_component_t::begin_component(editor_application_context_t& context, const char* header, ecs::component_type_t& component_type, const bool removable)
    {
        if (!component_type.has(context.selection.entity) || !ImGui::CollapsingHeader(header, ImGuiTreeNodeFlags_DefaultOpen))
        {
            return false;
        }

        ImGui::PushID(header);

        if (removable && ImGui::Button(remove_component_label))
        {
            context.scene_context.scene.remove_component(component_type, context.selection.entity);
            ImGui::PopID();
            return false;
        }

        return true;
    }

    bool object_inspector_component_t::begin_gizmo_component(editor_application_context_t& context, const char* header, ecs::component_type_t& component_type, const e_gizmo_targets gizmo_target)
    {
        if (!component_type.has(context.selection.entity))
        {
            return false;
        }

        const bool is_gizmo_target = context.selection.gizmo_target == gizmo_target;

        if (is_gizmo_target)
        {
            ImGui::PushStyleColor(ImGuiCol_Header, editor_colors_t::accent_subtle);
        }

        const bool is_open = ImGui::CollapsingHeader(header, ImGuiTreeNodeFlags_DefaultOpen);

        if (is_gizmo_target)
        {
            ImGui::PopStyleColor();
        }

        if (ImGui::IsItemClicked())
        {
            context.selection.gizmo_target = gizmo_target;
        }

        if (!is_open)
        {
            return false;
        }

        ImGui::PushID(header);

        if (ImGui::Button(remove_component_label))
        {
            context.scene_context.scene.remove_component(component_type, context.selection.entity);
            context.selection.gizmo_target = e_gizmo_targets::Transform2D;
            ImGui::PopID();
            return false;
        }

        return true;
    }

    void object_inspector_component_t::end_component()
    {
        ImGui::PopID();
    }

    void object_inspector_component_t::draw_name(editor_application_context_t& context)
    {
        components::name_component_t* name = context.scene_context.scene.get_name(context.selection.entity);

        if (name != nullptr)
        {
            ImGui::InputText(name_label, name->name, components::NAME_MAX_LENGTH);
        }
    }

    void object_inspector_component_t::draw_tags(editor_application_context_t& context)
    {
        if (!begin_component(context, tags_header, *context.scene_context.scene.tags_type, true))
        {
            return;
        }

        components::tags_component_t* tags = context.scene_context.scene.get_tags(context.selection.entity);

        ImGui::InputTextWithHint(tags_label, tags_hint, tags->tags, components::TAGS_MAX_LENGTH);

        end_component();
    }

    void object_inspector_component_t::draw_transform_2D(editor_application_context_t& context)
    {
        if (!begin_gizmo_component(context, transform_2D_header, *context.scene_context.scene.transform_2D_type, e_gizmo_targets::Transform2D))
        {
            return;
        }

        components::transform_2D_component_t* transform = context.scene_context.scene.get_transform_2D(context.selection.entity);

        ImGui::DragFloat2(position_label, &transform->position.x, position_drag_speed);
        ImGui::DragFloat(rotation_label, &transform->rotation, rotation_drag_speed);
        ImGui::DragFloat2(scale_label, &transform->scale.x, scale_drag_speed);

        if (context.scene_context.scene.has_parent(context.selection.entity))
        {
            const math::vec2_t world_position = context.scene_context.scene.get_world_transform_2D(context.selection.entity).position;
            ImGui::TextDisabled(world_position_format, world_position.x, world_position.y);
        }

        end_component();
    }

    void object_inspector_component_t::draw_sprite_2D(editor_application_context_t& context)
    {
        if (!begin_component(context, sprite_2D_header, *context.scene_context.scene.sprite_2D_type, true))
        {
            return;
        }

        components::sprite_2D_component_t* sprite = context.scene_context.scene.get_sprite_2D(context.selection.entity);

        path_field(texture_label, sprite->texture_path, components::TEXTURE_PATH_MAX_LENGTH);
        ImGui::DragFloat2(size_label, &sprite->size.x, size_drag_speed);
        ImGui::ColorEdit4(tint_label, &sprite->tint.x);

        path_field(material_label, sprite->material_path, components::MATERIAL_PATH_MAX_LENGTH);
        ImGui::Checkbox(lit_label, &sprite->is_lit);

        const rendering::e_material_shading_models shading_model = sprite->is_lit ? rendering::e_material_shading_models::Lit : rendering::e_material_shading_models::Unlit;

        if (sprite->material_path[0] != '\0' && context.engine_runtime->asset_manager.material_cache.is_failed(sprite->material_path, shading_model))
        {
            ImGui::PushTextWrapPos(0.0f);
            ImGui::TextColored(editor_colors_t::failure, "%s", material_failed_label);
            ImGui::PopTextWrapPos();
        }

        ImGui::BeginDisabled(sprite->material_path[0] == '\0');

        if (ImGui::Button(reload_material_label))
        {
            context.engine_runtime->asset_manager.material_cache.invalidate(sprite->material_path);
        }

        ImGui::SameLine();

        if (ImGui::Button(edit_material_label))
        {
            context.requests.file_to_edit = sprite->material_path;
        }

        ImGui::EndDisabled();

        end_component();
    }

    void object_inspector_component_t::draw_point_light_2D(editor_application_context_t& context)
    {
        if (!begin_component(context, point_light_2D_header, *context.scene_context.scene.point_light_2D_type, true))
        {
            return;
        }

        components::point_light_2D_component_t* light = context.scene_context.scene.get_point_light_2D(context.selection.entity);

        ImGui::ColorEdit3(color_label, &light->color.x);
        ImGui::DragFloat(radius_label, &light->radius, radius_drag_speed, radius_min, radius_max);
        ImGui::DragFloat(intensity_label, &light->intensity, intensity_drag_speed, intensity_min, intensity_max);
        ImGui::TextDisabled("%s", lit_sprites_hint_label);

        if (!context.scene_context.render_context.has_lighting())
        {
            ImGui::TextUnformatted(lighting_unavailable_label);
        }

        end_component();
    }

    void object_inspector_component_t::draw_label_2D(editor_application_context_t& context)
    {
        if (!begin_component(context, label_2D_header, *context.scene_context.scene.label_2D_type, true))
        {
            return;
        }

        components::label_2D_component_t* label = context.scene_context.scene.get_label_2D(context.selection.entity);

        ImGui::InputText(text_label, label->text, components::LABEL_TEXT_MAX_LENGTH);
        path_field(font_label, label->font_path, components::FONT_PATH_MAX_LENGTH);
        ImGui::SliderFloat(font_size_label, &label->font_size, font_size_min, font_size_max);
        ImGui::ColorEdit4(color_label, &label->color.x);

        end_component();
    }

    void object_inspector_component_t::draw_velocity_2D(editor_application_context_t& context)
    {
        if (!begin_component(context, velocity_2D_header, *context.scene_context.scene.velocity_2D_type, true))
        {
            return;
        }

        components::velocity_2D_component_t* velocity = context.scene_context.scene.get_velocity_2D(context.selection.entity);

        ImGui::DragFloat2(velocity_label, &velocity->velocity.x, scale_drag_speed);
        ImGui::DragFloat(speed_label, &velocity->speed, size_drag_speed);

        end_component();
    }

    void object_inspector_component_t::draw_box_collider_2D(editor_application_context_t& context)
    {
        if (!begin_gizmo_component(context, box_collider_2D_header, *context.scene_context.scene.box_collider_2D_type, e_gizmo_targets::BoxCollider2D))
        {
            return;
        }

        components::box_collider_2D_component_t* collider = context.scene_context.scene.get_box_collider_2D(context.selection.entity);

        ImGui::DragFloat2(offset_label, &collider->offset.x, position_drag_speed);
        ImGui::DragFloat2(size_label, &collider->size.x, size_drag_speed);

        end_component();
    }

    void object_inspector_component_t::draw_camera_2D(editor_application_context_t& context)
    {
        if (!begin_component(context, camera_2D_header, *context.scene_context.scene.camera_2D_type, true))
        {
            return;
        }

        components::camera_2D_component_t* camera = context.scene_context.scene.get_camera_2D(context.selection.entity);

        ImGui::DragFloat(zoom_label, &camera->zoom, zoom_drag_speed, zoom_min, zoom_max);
        ImGui::Checkbox(primary_label, &camera->is_primary);

        end_component();
    }

    void object_inspector_component_t::draw_audio_source(editor_application_context_t& context)
    {
        if (!begin_component(context, audio_source_header, *context.scene_context.scene.audio_source_type, true))
        {
            return;
        }

        components::audio_source_component_t* audio_source = context.scene_context.scene.get_audio_source(context.selection.entity);

        path_field(clip_label, audio_source->clip_path, components::AUDIO_CLIP_PATH_MAX_LENGTH);
        ImGui::SliderFloat(volume_label, &audio_source->volume, volume_min, volume_max);
        ImGui::Checkbox(loop_label, &audio_source->loop);
        ImGui::Checkbox(play_on_start_label, &audio_source->play_on_start);

        end_component();
    }

    void object_inspector_component_t::draw_script(editor_application_context_t& context)
    {
        if (!begin_component(context, script_header, *context.scene_context.scene.script_type, true))
        {
            return;
        }

        components::script_component_t* script = context.scene_context.scene.get_script(context.selection.entity);

        path_field(script_path_label, script->script_path, components::SCRIPT_PATH_MAX_LENGTH);

        ImGui::BeginDisabled(script->script_path[0] == '\0');

        if (ImGui::Button(edit_script_label))
        {
            context.requests.file_to_edit = script->script_path;
        }

        ImGui::EndDisabled();

        end_component();
    }

    void object_inspector_component_t::draw_add_component(editor_application_context_t& context)
    {
        ImGui::Separator();

        if (ImGui::Button(add_component_label))
        {
            ImGui::OpenPopup(add_component_popup);
        }

        if (!ImGui::BeginPopup(add_component_popup))
        {
            return;
        }

        scene::scene_t& scene      = context.scene_context.scene;
        const ecs::entity_t entity = context.selection.entity;

        if (!scene.tags_type->has(entity) && ImGui::MenuItem(tags_header))
        {
            scene.add_tags(entity);
        }

        if (!scene.transform_2D_type->has(entity) && ImGui::MenuItem(transform_2D_header))
        {
            scene.add_transform_2D(entity);
        }

        if (!scene.sprite_2D_type->has(entity) && ImGui::MenuItem(sprite_2D_header))
        {
            scene.add_sprite_2D(entity);
        }

        if (!scene.label_2D_type->has(entity) && ImGui::MenuItem(label_2D_header))
        {
            scene.add_label_2D(entity);
        }

        if (!scene.velocity_2D_type->has(entity) && ImGui::MenuItem(velocity_2D_header))
        {
            scene.add_velocity_2D(entity);
        }

        if (!scene.box_collider_2D_type->has(entity) && ImGui::MenuItem(box_collider_2D_header))
        {
            scene.add_box_collider_2D(entity);
        }

        if (!scene.point_light_2D_type->has(entity) && ImGui::MenuItem(point_light_2D_header))
        {
            scene.add_point_light_2D(entity);
        }

        if (!scene.camera_2D_type->has(entity) && ImGui::MenuItem(camera_2D_header))
        {
            scene.add_camera_2D(entity);
        }

        if (!scene.audio_source_type->has(entity) && ImGui::MenuItem(audio_source_header))
        {
            scene.add_audio_source(entity);
        }

        if (!scene.script_type->has(entity) && ImGui::MenuItem(script_header))
        {
            scene.add_script(entity);
        }

        ImGui::EndPopup();
    }
}

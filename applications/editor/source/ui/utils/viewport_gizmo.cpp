#include <byte-forge-editor/ui/utils/viewport_gizmo.h>

#include <byte-forge-engine/modules/math/utils/math_constants.h>
#include <byte-forge-engine/modules/math/utils/math_converter.h>

#include <cmath>

namespace byteforge::editor
{
    namespace
    {
        const char* const move_label   = "Move (W)";
        const char* const rotate_label = "Rotate (E)";
        const char* const scale_label  = "Scale (R)";

        const f32 axis_length       = 70.0f;
        const f32 handle_tolerance  = 7.0f;
        const f32 center_half_size  = 7.0f;
        const f32 scale_tip_size    = 5.0f;
        const f32 arrow_size        = 9.0f;
        const f32 ring_radius       = 60.0f;
        const f32 line_thickness    = 2.5f;
        const f32 minimum_scale     = 0.001f;
        const ImVec2 toolbar_offset = ImVec2(8.0f, 8.0f);

        const ImU32 x_axis_color   = IM_COL32(230, 80, 80, 255);
        const ImU32 y_axis_color   = IM_COL32(90, 200, 90, 255);
        const ImU32 both_color     = IM_COL32(240, 240, 240, 220);
        const ImU32 ring_color     = IM_COL32(90, 150, 255, 255);
        const ImU32 hover_color    = IM_COL32(255, 220, 60, 255);
        const ImU32 collider_color = IM_COL32(90, 230, 120, 200);

        f32 length(const ImVec2& vector)
        {
            return std::sqrt(vector.x * vector.x + vector.y * vector.y);
        }

        ImVec2 add(const ImVec2& first, const ImVec2& second)      { return ImVec2(first.x + second.x, first.y + second.y); }
        ImVec2 subtract(const ImVec2& first, const ImVec2& second) { return ImVec2(first.x - second.x, first.y - second.y); }
        ImVec2 multiply(const ImVec2& vector, const f32 scalar)     { return ImVec2(vector.x * scalar, vector.y * scalar); }
        f32 dot(const ImVec2& first, const ImVec2& second)          { return first.x * second.x + first.y * second.y; }

        ImVec2 normalize(const ImVec2& vector)
        {
            const f32 vector_length = length(vector);
            return vector_length > 0.0f ? multiply(vector, 1.0f / vector_length) : ImVec2(1.0f, 0.0f);
        }

        f32 distance_to_segment(const ImVec2& point, const ImVec2& start, const ImVec2& end)
        {
            const ImVec2 segment = subtract(end, start);
            const f32 segment_length_squared = dot(segment, segment);
            const f32 t = segment_length_squared > 0.0f ? std::fmax(0.0f, std::fmin(1.0f, dot(subtract(point, start), segment) / segment_length_squared)) : 0.0f;

            return length(subtract(point, add(start, multiply(segment, t))));
        }

        math::vec2_t rotate(const math::vec2_t& vector, const f32 degrees)
        {
            const f32 radians = math::math_converter_t::deg_to_rad(degrees);
            return math::vec2_t(vector.x * std::cos(radians) - vector.y * std::sin(radians), vector.x * std::sin(radians) + vector.y * std::cos(radians));
        }

        f32 safe_divide(const f32 value, const f32 divisor)
        {
            return divisor != 0.0f ? value / divisor : value;
        }

        bool mode_button(const char* label, const bool is_active)
        {
            if (is_active)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
            }

            const bool pressed = ImGui::Button(label);

            if (is_active)
            {
                ImGui::PopStyleColor();
            }

            return pressed;
        }
    }

    void viewport_gizmo_t::draw_toolbar(const ImVec2& viewport_origin)
    {
        ImGui::SetCursorScreenPos(add(viewport_origin, toolbar_offset));

        if (mode_button(move_label, mode == e_gizmo_modes::Move))
        {
            mode = e_gizmo_modes::Move;
        }

        ImGui::SameLine();

        if (mode_button(rotate_label, mode == e_gizmo_modes::Rotate))
        {
            mode = e_gizmo_modes::Rotate;
        }

        ImGui::SameLine();

        if (mode_button(scale_label, mode == e_gizmo_modes::Scale))
        {
            mode = e_gizmo_modes::Scale;
        }
    }

    void viewport_gizmo_t::process_shortcuts()
    {
        if (ImGui::GetIO().WantTextInput || is_dragging())
        {
            return;
        }

        if (ImGui::IsKeyPressed(ImGuiKey_W, false))
        {
            mode = e_gizmo_modes::Move;
        }
        else if (ImGui::IsKeyPressed(ImGuiKey_E, false))
        {
            mode = e_gizmo_modes::Rotate;
        }
        else if (ImGui::IsKeyPressed(ImGuiKey_R, false))
        {
            mode = e_gizmo_modes::Scale;
        }
    }

    void viewport_gizmo_t::update(editor_application_context_t& context, const ImVec2& viewport_origin, const ImVec2& viewport_size, const bool is_viewport_active, const bool was_viewport_clicked)
    {
        origin     = viewport_origin;
        size       = viewport_size;
        projection = context.sprite_renderer.get_projection();

        scene::scene_t& scene = context.scene_context.scene;

        if (!context.selection.has_entity || !scene.exists(context.selection.entity) || projection.values[0] == 0.0f || projection.values[5] == 0.0f)
        {
            active_handle = e_gizmo_handles::None;
            return;
        }

        const ecs::entity_t entity = context.selection.entity;
        const bool targets_collider = context.selection.gizmo_target == e_gizmo_targets::BoxCollider2D && scene.get_box_collider_2D(entity) != nullptr;

        if (!targets_collider && scene.get_transform_2D(entity) == nullptr)
        {
            active_handle = e_gizmo_handles::None;
            return;
        }

        const components::transform_2D_component_t world = scene.get_world_transform_2D(entity);
        math::vec2_t world_center = world.position;
        f32 axis_rotation = mode == e_gizmo_modes::Scale ? world.rotation : 0.0f;

        if (targets_collider)
        {
            world_center  = world.position + scene.get_box_collider_2D(entity)->offset;
            axis_rotation = 0.0f;
        }

        if (targets_collider && mode == e_gizmo_modes::Rotate)
        {
            active_handle = e_gizmo_handles::None;
            return;
        }

        const ImVec2 center = world_to_screen(world_center);
        const ImVec2 axis_x = multiply(normalize(world_direction_to_screen(world_center, rotate(math::vec2_t(1.0f, 0.0f), axis_rotation))), axis_length);
        const ImVec2 axis_y = multiply(normalize(world_direction_to_screen(world_center, rotate(math::vec2_t(0.0f, 1.0f), axis_rotation))), axis_length);
        const ImVec2 mouse  = ImGui::GetIO().MousePos;

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        draw_list->PushClipRect(origin, add(origin, size), true);

        if (targets_collider)
        {
            const math::vec2_t half_size = scene.get_box_collider_2D(entity)->size * 0.5f;
            draw_list->AddRect(world_to_screen(world_center - half_size), world_to_screen(world_center + half_size), collider_color, 0.0f, 0, 1.5f);
        }

        const e_gizmo_handles hovered = active_handle != e_gizmo_handles::None ? active_handle : find_handle(mouse, center, axis_x, axis_y);

        if (was_viewport_clicked && hovered != e_gizmo_handles::None)
        {
            active_handle             = hovered;
            drag_start_mouse          = mouse;
            drag_start_angle          = std::atan2(mouse.y - center.y, mouse.x - center.x);
            drag_start_world_position = world_center;

            if (const components::transform_2D_component_t* transform = scene.get_transform_2D(entity))
            {
                drag_start_transform = *transform;
            }

            if (const components::box_collider_2D_component_t* collider = scene.get_box_collider_2D(entity))
            {
                drag_start_collider = *collider;
            }
        }

        if (active_handle != e_gizmo_handles::None)
        {
            if (is_viewport_active && ImGui::IsMouseDown(ImGuiMouseButton_Left))
            {
                apply_drag(context, mouse, center, axis_x, axis_y);
            }
            else
            {
                active_handle = e_gizmo_handles::None;
            }
        }

        draw_handles(draw_list, center, axis_x, axis_y, hovered);
        draw_list->PopClipRect();
    }

    ImVec2 viewport_gizmo_t::world_to_screen(const math::vec2_t& world_position) const
    {
        const f32* values = projection.values;

        const f32 ndc_x = values[0] * world_position.x + values[4] * world_position.y + values[12];
        const f32 ndc_y = values[1] * world_position.x + values[5] * world_position.y + values[13];

        return ImVec2(origin.x + (ndc_x + 1.0f) * 0.5f * size.x, origin.y + (1.0f - ndc_y) * 0.5f * size.y);
    }

    math::vec2_t viewport_gizmo_t::screen_delta_to_world(const ImVec2& screen_delta) const
    {
        return math::vec2_t(screen_delta.x * 2.0f / (projection.values[0] * size.x), -screen_delta.y * 2.0f / (projection.values[5] * size.y));
    }

    ImVec2 viewport_gizmo_t::world_direction_to_screen(const math::vec2_t& world_position, const math::vec2_t& world_direction) const
    {
        return subtract(world_to_screen(world_position + world_direction), world_to_screen(world_position));
    }

    e_gizmo_handles viewport_gizmo_t::find_handle(const ImVec2& mouse, const ImVec2& center, const ImVec2& axis_x, const ImVec2& axis_y) const
    {
        if (mode == e_gizmo_modes::Rotate)
        {
            return std::fabs(length(subtract(mouse, center)) - ring_radius) <= handle_tolerance ? e_gizmo_handles::Ring : e_gizmo_handles::None;
        }

        if (std::fabs(mouse.x - center.x) <= center_half_size && std::fabs(mouse.y - center.y) <= center_half_size)
        {
            return e_gizmo_handles::Both;
        }

        if (distance_to_segment(mouse, center, add(center, axis_x)) <= handle_tolerance)
        {
            return e_gizmo_handles::AxisX;
        }

        if (distance_to_segment(mouse, center, add(center, axis_y)) <= handle_tolerance)
        {
            return e_gizmo_handles::AxisY;
        }

        return e_gizmo_handles::None;
    }

    void viewport_gizmo_t::draw_handles(ImDrawList* draw_list, const ImVec2& center, const ImVec2& axis_x, const ImVec2& axis_y, const e_gizmo_handles hovered) const
    {
        if (mode == e_gizmo_modes::Rotate)
        {
            draw_list->AddCircle(center, ring_radius, hovered == e_gizmo_handles::Ring ? hover_color : ring_color, 64, line_thickness);
            draw_list->AddCircleFilled(center, 3.0f, both_color);
            return;
        }

        const ImVec2 axes[]  = { axis_x, axis_y };
        const ImU32 colors[] = { x_axis_color, y_axis_color };
        const e_gizmo_handles handles[] = { e_gizmo_handles::AxisX, e_gizmo_handles::AxisY };

        for (usize index = 0; index < 2; index++)
        {
            const ImU32 color = hovered == handles[index] ? hover_color : colors[index];
            const ImVec2 tip  = add(center, axes[index]);

            draw_list->AddLine(center, tip, color, line_thickness);

            if (mode == e_gizmo_modes::Move)
            {
                const ImVec2 direction = normalize(axes[index]);
                const ImVec2 normal(-direction.y, direction.x);
                const ImVec2 base = subtract(tip, multiply(direction, arrow_size));

                draw_list->AddTriangleFilled(add(tip, multiply(direction, arrow_size * 0.5f)), add(base, multiply(normal, arrow_size * 0.5f)), subtract(base, multiply(normal, arrow_size * 0.5f)), color);
            }
            else
            {
                draw_list->AddRectFilled(subtract(tip, ImVec2(scale_tip_size, scale_tip_size)), add(tip, ImVec2(scale_tip_size, scale_tip_size)), color);
            }
        }

        const ImU32 center_color = hovered == e_gizmo_handles::Both ? hover_color : both_color;
        draw_list->AddRect(subtract(center, ImVec2(center_half_size, center_half_size)), add(center, ImVec2(center_half_size, center_half_size)), center_color, 0.0f, 0, line_thickness);
    }

    void viewport_gizmo_t::apply_drag(editor_application_context_t& context, const ImVec2& mouse, const ImVec2& center, const ImVec2& axis_x, const ImVec2& axis_y)
    {
        scene::scene_t& scene      = context.scene_context.scene;
        const ecs::entity_t entity = context.selection.entity;
        const bool targets_collider = context.selection.gizmo_target == e_gizmo_targets::BoxCollider2D;

        components::transform_2D_component_t* transform   = scene.get_transform_2D(entity);
        components::box_collider_2D_component_t* collider = scene.get_box_collider_2D(entity);

        const ImVec2 mouse_delta = subtract(mouse, drag_start_mouse);

        if (mode == e_gizmo_modes::Move)
        {
            math::vec2_t world_delta = screen_delta_to_world(mouse_delta);

            if (active_handle == e_gizmo_handles::AxisX)
            {
                world_delta.y = 0.0f;
            }
            else if (active_handle == e_gizmo_handles::AxisY)
            {
                world_delta.x = 0.0f;
            }

            if (targets_collider && collider != nullptr)
            {
                collider->offset = drag_start_collider.offset + world_delta;
                return;
            }

            if (transform == nullptr)
            {
                return;
            }

            math::vec2_t local_delta = world_delta;

            if (scene.has_parent(entity))
            {
                const components::transform_2D_component_t parent = scene.get_world_transform_2D(scene.get_parent(entity));
                const math::vec2_t unrotated = rotate(world_delta, -parent.rotation);

                local_delta = math::vec2_t(safe_divide(unrotated.x, parent.scale.x), safe_divide(unrotated.y, parent.scale.y));
            }

            transform->position = drag_start_transform.position + local_delta;
            return;
        }

        if (mode == e_gizmo_modes::Rotate)
        {
            if (transform == nullptr)
            {
                return;
            }

            const f32 angle = std::atan2(mouse.y - center.y, mouse.x - center.x);
            f32 delta_radians = angle - drag_start_angle;

            if (delta_radians > math::math_constants_t::PI)
            {
                delta_radians -= 2.0f * math::math_constants_t::PI;
            }
            else if (delta_radians < -math::math_constants_t::PI)
            {
                delta_radians += 2.0f * math::math_constants_t::PI;
            }

            const f32 screen_orientation = projection.values[5] < 0.0f ? 1.0f : -1.0f;
            transform->rotation = drag_start_transform.rotation + screen_orientation * delta_radians * 180.0f / math::math_constants_t::PI;
            return;
        }

        f32 factor_x = 1.0f;
        f32 factor_y = 1.0f;

        if (active_handle == e_gizmo_handles::Both)
        {
            const f32 start_distance = length(subtract(drag_start_mouse, center));
            const f32 factor         = start_distance > 0.0f ? length(subtract(mouse, center)) / start_distance : 1.0f;

            factor_x = factor;
            factor_y = factor;
        }
        else if (active_handle == e_gizmo_handles::AxisX)
        {
            factor_x = 1.0f + dot(mouse_delta, normalize(axis_x)) / axis_length;
        }
        else if (active_handle == e_gizmo_handles::AxisY)
        {
            factor_y = 1.0f + dot(mouse_delta, normalize(axis_y)) / axis_length;
        }

        if (targets_collider && collider != nullptr)
        {
            collider->size = math::vec2_t(std::fmax(minimum_scale, drag_start_collider.size.x * factor_x), std::fmax(minimum_scale, drag_start_collider.size.y * factor_y));
            return;
        }

        if (transform != nullptr)
        {
            transform->scale = math::vec2_t(drag_start_transform.scale.x * factor_x, drag_start_transform.scale.y * factor_y);
        }
    }
}

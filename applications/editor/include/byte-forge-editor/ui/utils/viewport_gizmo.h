#pragma once

#include <byte-forge-editor/editor_application_context.h>

#include <byte-forge-engine/modules/components/box_collider_2D_component.h>
#include <byte-forge-engine/modules/components/transform_2D_component.h>
#include <byte-forge-engine/modules/math/matrices/matrix4.h>
#include <byte-forge-engine/modules/math/vectors/vector2.h>

#include <imgui.h>

namespace byteforge::editor
{
    enum class e_gizmo_modes
    {
        Move,
        Rotate,
        Scale
    };

    enum class e_gizmo_handles
    {
        None,
        AxisX,
        AxisY,
        Both,
        Ring
    };

    class viewport_gizmo_t
    {
    public:
        e_gizmo_modes mode = e_gizmo_modes::Move;

        void draw_toolbar(const ImVec2& viewport_origin);
        void process_shortcuts();
        void update(editor_application_context_t& context, const ImVec2& viewport_origin, const ImVec2& viewport_size, const bool is_viewport_active, const bool was_viewport_clicked);

        bool is_dragging() const { return active_handle != e_gizmo_handles::None; }

    private:
        ImVec2 world_to_screen(const math::vec2_t& world_position) const;
        math::vec2_t screen_delta_to_world(const ImVec2& screen_delta) const;
        ImVec2 world_direction_to_screen(const math::vec2_t& world_position, const math::vec2_t& world_direction) const;
        e_gizmo_handles find_handle(const ImVec2& mouse, const ImVec2& center, const ImVec2& axis_x, const ImVec2& axis_y) const;
        void draw_handles(ImDrawList* draw_list, const ImVec2& center, const ImVec2& axis_x, const ImVec2& axis_y, const e_gizmo_handles hovered) const;
        void apply_drag(editor_application_context_t& context, const ImVec2& mouse, const ImVec2& center, const ImVec2& axis_x, const ImVec2& axis_y);

        math::mat4_t projection;
        ImVec2 origin;
        ImVec2 size;

        e_gizmo_handles active_handle = e_gizmo_handles::None;
        ImVec2 drag_start_mouse;
        f32 drag_start_angle = 0.0f;
        components::transform_2D_component_t drag_start_transform = {};
        components::box_collider_2D_component_t drag_start_collider = {};
        math::vec2_t drag_start_world_position;
    };
}

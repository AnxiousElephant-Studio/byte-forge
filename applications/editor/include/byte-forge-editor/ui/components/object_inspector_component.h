#pragma once

#include <byte-forge-editor/editor_application_context.h>

#include <byte-forge-engine/modules/ecs/component_type.h>

namespace byteforge::editor
{
    class object_inspector_component_t
    {
    public:
        void draw(editor_application_context_t& context);

    private:
        bool begin_component(editor_application_context_t& context, const char* header, ecs::component_type_t& component_type, const bool removable);
        bool begin_gizmo_component(editor_application_context_t& context, const char* header, ecs::component_type_t& component_type, const e_gizmo_targets gizmo_target);
        void end_component();

        void draw_name(editor_application_context_t& context);
        void draw_tags(editor_application_context_t& context);
        void draw_transform_2D(editor_application_context_t& context);
        void draw_sprite_2D(editor_application_context_t& context);
        void draw_label_2D(editor_application_context_t& context);
        void draw_velocity_2D(editor_application_context_t& context);
        void draw_box_collider_2D(editor_application_context_t& context);
        void draw_camera_2D(editor_application_context_t& context);
        void draw_point_light_2D(editor_application_context_t& context);
        void draw_audio_source(editor_application_context_t& context);
        void draw_script(editor_application_context_t& context);

        void draw_add_component(editor_application_context_t& context);
    };
}

#pragma once

#include <byte-forge-editor/editor_application_context.h>

#include <byte-forge-engine/api/types.h>

#include <string>
#include <vector>

namespace byteforge::editor
{
    class ecs_observer_component_t
    {
    public:
        ecs_observer_component_t() : validation_issues(), has_validated(false), validate_every_frame(false) {}

        void draw(editor_application_context_t& context);

    private:
        void draw_entities(editor_application_context_t& context);
        void draw_component_types(editor_application_context_t& context);
        void draw_systems(editor_application_context_t& context);
        void draw_system_actions(editor_application_context_t& context);
        void draw_validation(editor_application_context_t& context);

        std::vector<std::string> validation_issues;
        bool has_validated;
        bool validate_every_frame;
    };
}

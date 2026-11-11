#pragma once

#include <byte-forge-editor/editor_application_context.h>

#include <byte-forge-engine/modules/rendering/sprite/sprite_shader_descriptor.h>

namespace byteforge::editor
{
    class sprite_renderer_component_t
    {
    public:
        sprite_renderer_component_t() : edited_shader_descriptor(), has_loaded_shader_descriptor(false), is_last_reload_successful(true), has_requested_creation(false) {}

        void draw(editor_application_context_t& context);

    private:
        void reload_shader(editor_application_context_t& context);

        rendering::sprite_shader_descriptor_t edited_shader_descriptor;
        bool has_loaded_shader_descriptor;
        bool is_last_reload_successful;
        bool has_requested_creation;
    };
}

#include <byte-forge-editor/ui/components/sprite_renderer_component.h>
#include <byte-forge-editor/ui/utils/scoped_window.h>
#include <byte-forge-editor/ui/utils/drag_drop.h>
#include <byte-forge-editor/ui/utils/editor_colors.h>

#include <imgui.h>
#include <imgui_stdlib.h>

#include <string>

namespace byteforge::editor
{
    namespace
    {
        const char* const window_title             = "Sprite Renderer";
        const char* const shaders_header           = "Shaders";
        const char* const uniforms_header          = "Uniform names";
        const char* const vertex_shader_label      = "Vertex shader";
        const char* const fragment_shader_label    = "Fragment shader";
        const char* const edit_vertex_label        = "Edit##vertex_shader";
        const char* const edit_fragment_label      = "Edit##fragment_shader";
        const char* const projection_matrix_label  = "Projection matrix";
        const char* const model_matrix_label       = "Model matrix";
        const char* const tint_label               = "Tint";
        const char* const use_texture_label        = "Use texture";
        const char* const uv_rect_label            = "UV rect";
        const char* const texture_label            = "Texture";
        const char* const apply_label              = "Apply";
        const char* const create_label             = "Create";
        const char* const not_created_label        = "The sprite renderer is not created, so the scene is not drawn. Choose the shaders and press Create.";
        const char* const revert_label             = "Revert";
        const char* const reload_success_label     = "Shader loaded.";
        const char* const reload_failure_label     = "Shader failed to load, the previous one is still in use. See the Console Output.";


        void shader_path_field(editor_application_context_t& context, const char* label, const char* edit_label, std::string& path)
        {
            ImGui::InputText(label, &path);
            accept_asset_path_drop(path);

            ImGui::SameLine();
            ImGui::BeginDisabled(path.empty());

            if (ImGui::Button(edit_label))
            {
                context.requests.file_to_edit = path;
            }

            ImGui::EndDisabled();
        }
    }

    void sprite_renderer_component_t::draw(editor_application_context_t& context)
    {
        if (!has_loaded_shader_descriptor)
        {
            edited_shader_descriptor = context.sprite_renderer.is_initialized()
                ? context.sprite_renderer.get_shader_descriptor()
                : rendering::sprite_shader_descriptor_t(context.current_project.settings.sprite_vertex_shader.c_str(), context.current_project.settings.sprite_fragment_shader.c_str());

            has_loaded_shader_descriptor = true;
        }

        if (context.requests.is_sprite_shader_reload_requested)
        {
            context.requests.is_sprite_shader_reload_requested = false;

            if (context.sprite_renderer.is_initialized())
            {
                reload_shader(context);
            }
        }

        const scoped_window_t window(window_title);

        if (!window.is_visible())
        {
            return;
        }

        if (ImGui::CollapsingHeader(shaders_header, ImGuiTreeNodeFlags_DefaultOpen))
        {
            shader_path_field(context, vertex_shader_label, edit_vertex_label, edited_shader_descriptor.vertex_shader_path);
            shader_path_field(context, fragment_shader_label, edit_fragment_label, edited_shader_descriptor.fragment_shader_path);
        }

        if (ImGui::CollapsingHeader(uniforms_header, ImGuiTreeNodeFlags_DefaultOpen))
        {
            rendering::sprite_shader_uniform_names_t& uniform_names = edited_shader_descriptor.uniform_names;

            ImGui::InputText(projection_matrix_label, &uniform_names.projection_matrix);
            ImGui::InputText(model_matrix_label, &uniform_names.model_matrix);
            ImGui::InputText(tint_label, &uniform_names.tint);
            ImGui::InputText(use_texture_label, &uniform_names.use_texture);
            ImGui::InputText(uv_rect_label, &uniform_names.uv_rect);
            ImGui::InputText(texture_label, &uniform_names.texture);
        }

        ImGui::Separator();

        const bool is_created = context.sprite_renderer.is_initialized();

        if (ImGui::Button(is_created ? apply_label : create_label))
        {
            has_requested_creation = true;
            reload_shader(context);
        }

        if (is_created)
        {
            ImGui::SameLine();

            if (ImGui::Button(revert_label))
            {
                edited_shader_descriptor = context.sprite_renderer.get_shader_descriptor();
            }
        }

        if (has_requested_creation)
        {
            ImGui::SameLine();
            ImGui::TextColored(is_last_reload_successful ? editor_colors_t::success : editor_colors_t::failure, "%s", is_last_reload_successful ? reload_success_label : reload_failure_label);
        }

        if (!context.sprite_renderer.is_initialized())
        {
            ImGui::PushTextWrapPos();
            ImGui::TextUnformatted(not_created_label);
            ImGui::PopTextWrapPos();
        }
    }

    void sprite_renderer_component_t::reload_shader(editor_application_context_t& context)
    {
        is_last_reload_successful = context.sprite_renderer.reload_shader(edited_shader_descriptor);
    }
}

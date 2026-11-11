#include <byte-forge-engine/modules/rendering/sprite/sprite_renderer.h>
#include <byte-forge-engine/modules/rendering/sprite/sprite_material_template.h>
#include <byte-forge-engine/modules/rendering/opengl/state/opengl_render_state.h>
#include <byte-forge-engine/modules/logging/log_macros.h>

#include <glad/gl.h>

namespace byteforge::rendering
{
    namespace
    {
        const f32 near_plane = -1.0f;
        const f32 far_plane  = 1.0f;
        const f32 half       = 0.5f;

        const u32 texture_slot   = 0;
        const u32 light_map_slot = 1;

        const char* const default_lit_material_label = "<default lit sprite material>";

        const math::vec4_t full_uv_rect(0.0f, 0.0f, 1.0f, 1.0f);

        const i32 missing_uniform_location = -1;

        const char* const missing_uniform_warning_format = "Could not find uniform: '%s' in shader program with vertex shader at path: '%s' and fragment shader at path: '%s'. Cause: the uniform is not declared or not used by the shader.";
        const char* const reload_error_format            = "Could not reload sprite shader with vertex shader at path: '%s' and fragment shader at path: '%s'. Cause: compilation or linking failed, the previous shader is kept.";

        i32 find_uniform_location(const opengl_shader_t& shader, const sprite_shader_descriptor_t& descriptor, const std::string& uniform_name)
        {
            const i32 location = shader.get_uniform_location(uniform_name.c_str());

            if (location == missing_uniform_location)
            {
                LOG_WARNING(missing_uniform_warning_format, uniform_name.c_str(), descriptor.vertex_shader_path.c_str(), descriptor.fragment_shader_path.c_str());
            }

            return location;
        }
    }

    bool sprite_renderer_t::initialize(const sprite_shader_descriptor_t& descriptor)
    {
        if (!shader.create(descriptor.vertex_shader_path.c_str(), descriptor.fragment_shader_path.c_str()))
        {
            return false;
        }

        if (!quad.create_quad())
        {
            shader.dispose();
            return false;
        }

        shader_descriptor = descriptor;
        default_lit_material.load_from_source(sprite_material_template, default_lit_material_label, e_material_shading_models::Lit);
        resolve_uniform_locations();

        return true;
    }

    bool sprite_renderer_t::reload_shader(const sprite_shader_descriptor_t& descriptor)
    {
        if (!is_initialized())
        {
            return initialize(descriptor);
        }

        opengl_shader_t reloaded_shader;

        if (!reloaded_shader.create(descriptor.vertex_shader_path.c_str(), descriptor.fragment_shader_path.c_str()))
        {
            LOG_ERROR(reload_error_format, descriptor.vertex_shader_path.c_str(), descriptor.fragment_shader_path.c_str());
            return false;
        }

        shader.dispose();
        shader            = reloaded_shader;
        shader_descriptor = descriptor;
        resolve_uniform_locations();

        return true;
    }

    const sprite_shader_descriptor_t& sprite_renderer_t::get_shader_descriptor() const
    {
        return shader_descriptor;
    }

    bool sprite_renderer_t::is_initialized() const
    {
        return shader.program_id != 0;
    }

    void sprite_renderer_t::resolve_uniform_locations()
    {
        const sprite_shader_uniform_names_t& uniform_names = shader_descriptor.uniform_names;

        projection_location  = find_uniform_location(shader, shader_descriptor, uniform_names.projection_matrix);
        model_location       = find_uniform_location(shader, shader_descriptor, uniform_names.model_matrix);
        tint_location        = find_uniform_location(shader, shader_descriptor, uniform_names.tint);
        use_texture_location = find_uniform_location(shader, shader_descriptor, uniform_names.use_texture);
        uv_rect_location     = find_uniform_location(shader, shader_descriptor, uniform_names.uv_rect);

        shader.bind();
        shader.set_int(find_uniform_location(shader, shader_descriptor, uniform_names.texture), static_cast<i32>(texture_slot));
    }

    void sprite_renderer_t::begin(const math::vec2i_t& viewport_size, const math::vec2_t& camera_position, const f32 zoom, const bool use_camera)
    {
        opengl_render_state_t::enable_blending();
        opengl_render_state_t::use_alpha_blending();

        const f32 width  = static_cast<f32>(viewport_size.x);
        const f32 height = static_cast<f32>(viewport_size.y);

        f32 left   = 0.0f;
        f32 right  = width;
        f32 top    = 0.0f;
        f32 bottom = height;

        if (use_camera && zoom > 0.0f)
        {
            const f32 half_width  = width * half / zoom;
            const f32 half_height = height * half / zoom;

            left   = camera_position.x - half_width;
            right  = camera_position.x + half_width;
            top    = camera_position.y - half_height;
            bottom = camera_position.y + half_height;
        }

        shader.bind();
        projection = math::mat4_t::orthographic(left, right, bottom, top, near_plane, far_plane);
        shader.set_matrix_4(projection_location, projection);
    }

    void sprite_renderer_t::resume() const
    {
        opengl_render_state_t::enable_blending();
        opengl_render_state_t::use_alpha_blending();

        shader.bind();
    }

    void sprite_renderer_t::set_material_frame(f32 time_in_seconds, const opengl_texture_t* frame_light_map)
    {
        material_time = time_in_seconds;
        light_map     = frame_light_map;
    }

    void sprite_renderer_t::draw_with_material(material_t& material, const opengl_texture_t* texture, const math::vec2_t& position, const math::vec2_t& size, f32 rotation_degrees, const math::vec4_t& tint)
    {
        const math::mat4_t model = math::mat4_t::identity()
            .translate(position.x, position.y, 0.0f)
            .rotate_z(rotation_degrees)
            .scale(size.x, size.y, 1.0f);

        material.bind();
        material.set_projection(projection);
        material.set_model(model);
        material.set_time(material_time);
        material.set_vector_4("u_tint", tint);
        material.set_int("u_use_texture", texture != nullptr ? 1 : 0);

        if (texture != nullptr)
        {
            material.set_texture("u_texture", *texture, texture_slot);
        }

        if (material.get_shading_model() == e_material_shading_models::Lit && light_map != nullptr)
        {
            material.set_light_map(*light_map, light_map_slot);
        }

        quad.draw();

        shader.bind();
    }

    void sprite_renderer_t::draw(const opengl_texture_t* texture, const math::vec2_t& position, const math::vec2_t& size, f32 rotation_degrees, const math::vec4_t& tint)
    {
        draw_region(texture, position, size, rotation_degrees, tint, full_uv_rect);
    }

    void sprite_renderer_t::draw_region(const opengl_texture_t* texture, const math::vec2_t& position, const math::vec2_t& size, f32 rotation_degrees, const math::vec4_t& tint, const math::vec4_t& uv_rect)
    {
        const math::mat4_t model = math::mat4_t::identity()
            .translate(position.x, position.y, 0.0f)
            .rotate_z(rotation_degrees)
            .scale(size.x, size.y, 1.0f);

        shader.set_matrix_4(model_location, model);
        shader.set_vector_4(tint_location, tint);
        shader.set_vector_4(uv_rect_location, uv_rect);
        shader.set_bool(use_texture_location, texture != nullptr);

        if (texture != nullptr)
        {
            texture->bind(texture_slot);
        }

        quad.draw();
    }

    void sprite_renderer_t::draw_text(const font_t& font, const char* text, const math::vec2_t& position, const math::vec4_t& color)
    {
        const f32 atlas_width  = static_cast<f32>(font.atlas.size.x);
        const f32 atlas_height = static_cast<f32>(font.atlas.size.y);

        f32 pen_x = position.x;
        const f32 baseline_y = position.y + font.size;

        for (const char* character = text; *character != '\0'; character++)
        {
            const u32 code = static_cast<u8>(*character);

            if (code < FONT_FIRST_CHARACTER || code >= FONT_FIRST_CHARACTER + FONT_CHARACTER_COUNT)
            {
                continue;
            }

            const font_glyph_t& glyph = font.glyphs[code - FONT_FIRST_CHARACTER];

            const f32 glyph_width  = static_cast<f32>(glyph.x1 - glyph.x0);
            const f32 glyph_height = static_cast<f32>(glyph.y1 - glyph.y0);

            if (glyph_width > 0.0f && glyph_height > 0.0f)
            {
                const math::vec2_t glyph_center(pen_x + glyph.x_offset + glyph_width * half, baseline_y + glyph.y_offset + glyph_height * half);
                const math::vec4_t uv_rect(glyph.x0 / atlas_width, glyph.y0 / atlas_height, glyph_width / atlas_width, glyph_height / atlas_height);

                draw_region(&font.atlas, glyph_center, math::vec2_t(glyph_width, glyph_height), 0.0f, color, uv_rect);
            }

            pen_x += glyph.x_advance;
        }
    }

    void sprite_renderer_t::dispose()
    {
        default_lit_material.dispose();
        quad.dispose();
        shader.dispose();
    }
}

#pragma once

#include <byte-forge-engine/modules/rendering/rendering_api.h>
#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/math/matrices/matrix4.h>
#include <byte-forge-engine/modules/math/vectors/vector2.h>
#include <byte-forge-engine/modules/math/vectors/vector2i.h>
#include <byte-forge-engine/modules/math/vectors/vector4.h>
#include <byte-forge-engine/modules/rendering/font/font.h>
#include <byte-forge-engine/modules/rendering/material/material.h>
#include <byte-forge-engine/modules/rendering/opengl/mesh/opengl_mesh.h>
#include <byte-forge-engine/modules/rendering/opengl/shader/opengl_shader.h>
#include <byte-forge-engine/modules/rendering/opengl/texture/opengl_texture.h>
#include <byte-forge-engine/modules/rendering/sprite/sprite_shader_descriptor.h>

namespace byteforge::rendering
{
    class BF_RENDERING_API sprite_renderer_t
    {
    public:
        sprite_renderer_t()
            : shader()
            , quad()
            , shader_descriptor()
            , projection()
            , material_time(0.0f)
            , light_map(nullptr)
            , default_lit_material()
            , projection_location(-1)
            , model_location(-1)
            , tint_location(-1)
            , use_texture_location(-1)
            , uv_rect_location(-1)
        {
        }

        bool initialize(const sprite_shader_descriptor_t& descriptor);
        void dispose();

        bool reload_shader(const sprite_shader_descriptor_t& descriptor);

        void begin(const math::vec2i_t& viewport_size, const math::vec2_t& camera_position, const f32 zoom, const bool use_camera);

        void resume() const;

        void draw(const opengl_texture_t* texture, const math::vec2_t& position, const math::vec2_t& size, f32 rotation_degrees, const math::vec4_t& tint);
        void draw_region(const opengl_texture_t* texture, const math::vec2_t& position, const math::vec2_t& size, f32 rotation_degrees, const math::vec4_t& tint, const math::vec4_t& uv_rect);
        void draw_text(const font_t& font, const char* text, const math::vec2_t& position, const math::vec4_t& color);

        void set_material_frame(f32 time_in_seconds, const opengl_texture_t* frame_light_map);
        void draw_with_material(material_t& material, const opengl_texture_t* texture, const math::vec2_t& position, const math::vec2_t& size, f32 rotation_degrees, const math::vec4_t& tint);

        material_t* get_default_lit_material() { return default_lit_material.is_loaded() ? &default_lit_material : nullptr; }

        const math::mat4_t& get_projection() const { return projection; }
        const sprite_shader_descriptor_t& get_shader_descriptor() const;
        bool is_initialized() const;

    private:
        void resolve_uniform_locations();

        opengl_shader_t shader;
        opengl_mesh_t quad;
        sprite_shader_descriptor_t shader_descriptor;
        math::mat4_t projection;
        f32 material_time;
        const opengl_texture_t* light_map;
        material_t default_lit_material;
        i32 projection_location;
        i32 model_location;
        i32 tint_location;
        i32 use_texture_location;
        i32 uv_rect_location;
    };
}

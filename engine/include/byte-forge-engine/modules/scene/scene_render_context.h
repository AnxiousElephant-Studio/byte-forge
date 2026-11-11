#pragma once

#include <byte-forge-engine/modules/assets/asset_manager.h>
#include <byte-forge-engine/modules/math/vectors/vector2i.h>
#include <byte-forge-engine/modules/math/vectors/vector4.h>
#include <byte-forge-engine/modules/rendering/lighting/point_light_renderer.h>
#include <byte-forge-engine/modules/rendering/opengl/framebuffer/opengl_framebuffer.h>
#include <byte-forge-engine/modules/rendering/sprite/sprite_renderer.h>

namespace byteforge::scene
{
    struct scene_render_context_t
    {
    public:
        rendering::sprite_renderer_t* sprite_renderer;
        assets::asset_manager_t* asset_manager;
        math::vec2i_t target_size;

        rendering::point_light_renderer_t* point_light_renderer;
        rendering::opengl_framebuffer_t* light_map_framebuffer;
        const rendering::opengl_framebuffer_t* target_framebuffer;
        math::vec4_t ambient_light_color;

        f64 elapsed_time;

        scene_render_context_t()
            : sprite_renderer(nullptr)
            , asset_manager(nullptr)
            , target_size()
            , point_light_renderer(nullptr)
            , light_map_framebuffer(nullptr)
            , target_framebuffer(nullptr)
            , ambient_light_color(0.35f, 0.35f, 0.40f, 1.0f)
            , elapsed_time(0.0)
        {
        }

        bool is_ready() const
        {
            return sprite_renderer != nullptr && sprite_renderer->is_initialized() && asset_manager != nullptr && target_size.x > 0 && target_size.y > 0;
        }

        bool has_lighting() const
        {
            return point_light_renderer != nullptr && point_light_renderer->is_initialized() && light_map_framebuffer != nullptr;
        }
    };
}

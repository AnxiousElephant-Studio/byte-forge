#pragma once

#include <byte-forge-engine/modules/rendering/rendering_api.h>
#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/math/matrices/matrix4.h>
#include <byte-forge-engine/modules/rendering/lighting/point_light.h>
#include <byte-forge-engine/modules/rendering/opengl/mesh/opengl_mesh.h>
#include <byte-forge-engine/modules/rendering/opengl/shader/opengl_shader.h>

namespace byteforge::rendering
{
    class BF_RENDERING_API point_light_renderer_t
    {
    public:
        point_light_renderer_t()
            : shader()
            , quad()
            , projection_location(-1)
            , model_location(-1)
            , light_position_location(-1)
            , light_radius_location(-1)
            , light_color_location(-1)
            , light_intensity_location(-1)
        {
        }

        bool initialize();
        void dispose();

        void begin(const math::mat4_t& projection) const;
        void draw(const point_light_t& light) const;

        bool is_initialized() const { return shader.program_id != 0; }

    private:
        opengl_shader_t shader;
        opengl_mesh_t quad;
        i32 projection_location;
        i32 model_location;
        i32 light_position_location;
        i32 light_radius_location;
        i32 light_color_location;
        i32 light_intensity_location;
    };
}

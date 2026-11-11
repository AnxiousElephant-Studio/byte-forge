#pragma once

#include <byte-forge-engine/modules/rendering/rendering_api.h>
#include <byte-forge-engine/api/types.h>

namespace byteforge::rendering
{
    struct BF_RENDERING_API opengl_mesh_t
    {
    public:
        u32 vertex_array_id;
        u32 vertex_buffer_id;
        u32 index_buffer_id;
        u32 index_count;

        opengl_mesh_t() : vertex_array_id(0), vertex_buffer_id(0), index_buffer_id(0), index_count(0) {}

        bool create(const f32* vertices, usize vertex_float_count,
                    const u32* indices, usize index_count,
                    const u32* attribute_sizes, usize attribute_count);
        bool create_quad();
        void dispose();

        void draw() const;
    };
}

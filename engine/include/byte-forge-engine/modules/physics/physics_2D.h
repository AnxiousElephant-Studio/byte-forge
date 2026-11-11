#pragma once

#include <byte-forge-engine/modules/physics/physics_api.h>
#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/math/vectors/vector2.h>

namespace byteforge::physics
{
    struct BF_PHYSICS_API physics_2D_t
    {
    public:
        static math::vec2_t move(const math::vec2_t& position, const math::vec2_t& velocity, const f32 speed, const f32 delta_time);
        static bool check_aabb_collision(const math::vec2_t& a_position, const math::vec2_t& a_size, const math::vec2_t& b_position, const math::vec2_t& b_size);
    };
}

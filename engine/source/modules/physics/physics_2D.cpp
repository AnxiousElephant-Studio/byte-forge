#include <byte-forge-engine/modules/physics/physics_2D.h>

#include <byte-forge-engine/modules/math/utils/math_constants.h>

namespace byteforge::physics
{
    math::vec2_t physics_2D_t::move(const math::vec2_t& position, const math::vec2_t& velocity, const f32 speed, const f32 delta_time)
    {
        return position + velocity * speed * delta_time;
    }

    bool physics_2D_t::check_aabb_collision(const math::vec2_t& a_position, const math::vec2_t& a_size, const math::vec2_t& b_position, const math::vec2_t& b_size)
    {
        const bool collision_x =
            a_position.x - a_size.x * math::math_constants_t::HALF <= b_position.x + b_size.x * math::math_constants_t::HALF &&
            a_position.x + a_size.x * math::math_constants_t::HALF >= b_position.x - b_size.x * math::math_constants_t::HALF;

        const bool collision_y =
            a_position.y - a_size.y * math::math_constants_t::HALF <= b_position.y + b_size.y * math::math_constants_t::HALF &&
            a_position.y + a_size.y * math::math_constants_t::HALF >= b_position.y - b_size.y * math::math_constants_t::HALF;

        return collision_x && collision_y;
    }
}

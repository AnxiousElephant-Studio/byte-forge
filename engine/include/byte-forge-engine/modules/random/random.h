#pragma once

#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/math/vectors/vector2.h>

#include <random>

namespace byteforge::random
{
    struct random_t
    {
    public:
        random_t() : generator(std::random_device{}()) {}

        i32 range_int(i32 min, i32 max)
        {
            std::uniform_int_distribution<i32> distribution(min, max);
            return distribution(generator);
        }

        f32 range_float(f32 min, f32 max)
        {
            std::uniform_real_distribution<f32> distribution(min, max);
            return distribution(generator);
        }

        math::vec2_t range_vec2(const math::vec2_t& min, const math::vec2_t& max)
        {
            return math::vec2_t(range_float(min.x, max.x), range_float(min.y, max.y));
        }

        f32 next_float()
        {
            return range_float(0.0f, 1.0f);
        }

        i32 next_int()
        {
            return range_int(0, 1);
        }

        math::vec2_t next_vec2()
        {
            return range_vec2(math::vec2_t::zero(), math::vec2_t::one());
        }

    private:
        std::mt19937 generator;
    };
}

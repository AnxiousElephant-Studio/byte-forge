#pragma once

#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/math/utils/math_converter.h>
#include <byte-forge-engine/modules/math/vectors/vector3.h>

#include <cmath>

namespace byteforge::math
{
    struct mat4_t
    {
    public:
        f32 values[16];

        constexpr mat4_t() : values{} {}

        static constexpr mat4_t identity()
        {
            mat4_t result = {};

            result.values[0] = 1.0f;
            result.values[5] = 1.0f;
            result.values[10] = 1.0f;
            result.values[15] = 1.0f;

            return result;
        }

        static constexpr mat4_t orthographic(
            f32 left,
            f32 right,
            f32 bottom,
            f32 top,
            f32 near_plane,
            f32 far_plane
        )
        {
            mat4_t result = {};

            result.values[0] = 2.0f / (right - left);
            result.values[5] = 2.0f / (top - bottom);
            result.values[10] = -2.0f / (far_plane - near_plane);
            result.values[15] = 1.0f;

            result.values[12] = -(right + left) / (right - left);
            result.values[13] = -(top + bottom) / (top - bottom);
            result.values[14] = -(far_plane + near_plane) / (far_plane - near_plane);

            return result;
        }

        constexpr mat4_t translate(f32 x, f32 y, f32 z) const
        {
            mat4_t translation = identity();

            translation.values[12] = x;
            translation.values[13] = y;
            translation.values[14] = z;

            return multiply(translation);
        }

        constexpr mat4_t translate(const vec3_t& vector) const
        {
            return translate(vector.x, vector.y, vector.z);
        }

        constexpr mat4_t scale(f32 size_x, f32 size_y, f32 size_z) const
        {
            mat4_t scaling = identity();

            scaling.values[0] = size_x;
            scaling.values[5] = size_y;
            scaling.values[10] = size_z;

            return multiply(scaling);
        }

        mat4_t rotate_z(f32 degrees) const
        {
            const f32 radians = math_converter_t::deg_to_rad(degrees);

            const f32 cosine = std::cos(radians);
            const f32 sine = std::sin(radians);

            mat4_t rotation = identity();

            rotation.values[0] = cosine;
            rotation.values[1] = sine;
            rotation.values[4] = -sine;
            rotation.values[5] = cosine;

            return multiply(rotation);
        }

        constexpr mat4_t multiply(const mat4_t& right) const
        {
            mat4_t result = {};

            for (i32 column = 0; column < 4; ++column)
            {
                for (i32 row = 0; row < 4; ++row)
                {
                    f32 sum = 0.0f;

                    for (i32 index = 0; index < 4; ++index)
                    {
                        sum += values[index * 4 + row] * right.values[column * 4 + index];
                    }

                    result.values[column * 4 + row] = sum;
                }
            }

            return result;
        }
    };
}

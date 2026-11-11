#pragma once

#include <byte-forge-engine/api/types.h>

#include <cmath>

namespace byteforge::math
{
    struct vec3_t
    {
    public:
        f32 x, y, z;

        constexpr vec3_t()                    : x(0), y(0), z(0) {}
        constexpr vec3_t(f32 x, f32 y, f32 z) : x(x), y(y), z(z) {}

        constexpr f32 r() const { return x; }
        constexpr f32 g() const { return y; }
        constexpr f32 b() const { return z; }

        constexpr f32  operator[](usize index) const { return index == 0 ? x : index == 1 ? y : z; }
        constexpr f32& operator[](usize index)       { return index == 0 ? x : index == 1 ? y : z; }

        constexpr vec3_t operator+(const vec3_t& other) const { return { x + other.x, y + other.y, z + other.z }; }
        constexpr vec3_t operator-(const vec3_t& other) const { return { x - other.x, y - other.y, z - other.z }; }
        constexpr vec3_t operator*(f32 scalar)          const { return { x * scalar, y * scalar, z * scalar }; }
        constexpr vec3_t operator/(f32 scalar)          const { return { x / scalar, y / scalar, z / scalar }; }

        constexpr vec3_t& operator+=(const vec3_t& other) { x += other.x; y += other.y; z += other.z; return *this; }
        constexpr vec3_t& operator-=(const vec3_t& other) { x -= other.x; y -= other.y; z -= other.z; return *this; }
        constexpr vec3_t& operator*=(f32 scalar)          { x *= scalar; y *= scalar; z *= scalar; return *this; }
        constexpr vec3_t& operator/=(f32 scalar)          { x /= scalar; y /= scalar; z /= scalar; return *this; }

        constexpr bool operator==(const vec3_t& other) const { return x == other.x && y == other.y && z == other.z; }

        f32 length() const { return std::sqrt(length_squared()); }

        constexpr f32 length_squared() const { return (x*x + y*y + z*z); }

        constexpr f32 dot(const vec3_t& other) const { return x*other.x + y*other.y + z*other.z; }

        constexpr vec3_t cross(const vec3_t& other) const
        {
            return {
                y * other.z - z * other.y,
                z * other.x - x * other.z,
                x * other.y - y * other.x
            };
        }

        vec3_t normalized() const
        {
            const f32 vector_length = length();

            if (vector_length == 0.0f)
            {
                return zero();
            }

            return *this / vector_length;
        }

        f32 distance(const vec3_t& other) const
        {
            return (*this - other).length();
        }

        constexpr f32 distance_squared(const vec3_t& other) const
        {
            return (*this - other).length_squared();
        }

        constexpr static vec3_t zero()  { return {  0,  0,  0 }; }
        constexpr static vec3_t one()   { return {  1,  1,  1 }; }
        constexpr static vec3_t up()    { return {  0,  1,  0 }; }
        constexpr static vec3_t down()  { return {  0, -1,  0 }; }
        constexpr static vec3_t left()  { return { -1,  0,  0 }; }
        constexpr static vec3_t right() { return {  1,  0,  0 }; }
    };

    constexpr vec3_t operator*(f32 scalar, const vec3_t& vector) { return vector * scalar; }
}

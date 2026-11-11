#pragma once

#include <byte-forge-engine/api/types.h>

#include <cmath>

namespace byteforge::math
{
    struct vec4_t
    {
    public:
        f32 x, y, z, w;

        constexpr vec4_t()                           : x(0), y(0), z(0), w(0) {}
        constexpr vec4_t(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w) {}

        constexpr f32 r() const { return x; }
        constexpr f32 g() const { return y; }
        constexpr f32 b() const { return z; }
        constexpr f32 a() const { return w; }

        constexpr f32  operator[](usize index) const { return index == 0 ? x : index == 1 ? y : index == 2 ? z : w; }
        constexpr f32& operator[](usize index)       { return index == 0 ? x : index == 1 ? y : index == 2 ? z : w; }

        constexpr vec4_t operator+(const vec4_t& other) const { return { x + other.x, y + other.y, z + other.z, w + other.w }; }
        constexpr vec4_t operator-(const vec4_t& other) const { return { x - other.x, y - other.y, z - other.z, w - other.w }; }
        constexpr vec4_t operator*(f32 scalar)          const { return { x * scalar, y * scalar, z * scalar, w * scalar }; }
        constexpr vec4_t operator/(f32 scalar)          const { return { x / scalar, y / scalar, z / scalar, w / scalar }; }

        constexpr vec4_t& operator+=(const vec4_t& other) { x += other.x; y += other.y; z += other.z; w += other.w; return *this; }
        constexpr vec4_t& operator-=(const vec4_t& other) { x -= other.x; y -= other.y; z -= other.z; w -= other.w; return *this; }
        constexpr vec4_t& operator*=(f32 scalar)          { x *= scalar; y *= scalar; z *= scalar; w *= scalar; return *this; }
        constexpr vec4_t& operator/=(f32 scalar)          { x /= scalar; y /= scalar; z /= scalar; w /= scalar; return *this; }

        constexpr bool operator==(const vec4_t& other) const { return x == other.x && y == other.y && z == other.z && w == other.w; }

        f32 length() const { return std::sqrt(length_squared()); }

        constexpr f32 length_squared() const { return (x*x + y*y + z*z + w*w); }

        constexpr f32 dot(const vec4_t& other) const { return x*other.x + y*other.y + z*other.z + w*other.w; }

        vec4_t normalized() const
        {
            const f32 vector_length = length();

            if (vector_length == 0.0f)
            {
                return zero();
            }

            return *this / vector_length;
        }

        f32 distance(const vec4_t& other) const
        {
            return (*this - other).length();
        }

        constexpr f32 distance_squared(const vec4_t& other) const
        {
            return (*this - other).length_squared();
        }

        constexpr static vec4_t zero() { return { 0, 0, 0, 0 }; }
        constexpr static vec4_t one()  { return { 1, 1, 1, 1 }; }
    };

    constexpr vec4_t operator*(f32 scalar, const vec4_t& vector) { return vector * scalar; }
}

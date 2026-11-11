#pragma once

#include <byte-forge-engine/api/types.h>

#include <cmath>

namespace byteforge::math
{
    struct vec2_t
    {
    public:
        f32 x, y;

        constexpr vec2_t()             : x(0), y(0) {}
        constexpr vec2_t(f32 x, f32 y) : x(x), y(y) {}

        constexpr f32 width()  const { return x; }
        constexpr f32 height() const { return y; }
        constexpr f32 r()      const { return x; }
        constexpr f32 g()      const { return y; }
        constexpr f32 u()      const { return x; }
        constexpr f32 v()      const { return y; }

        constexpr f32  operator[](usize index) const { return index == 0 ? x : y; }
        constexpr f32& operator[](usize index)       { return index == 0 ? x : y; }

        constexpr vec2_t operator+(const vec2_t& other) const { return { x + other.x, y + other.y }; }
        constexpr vec2_t operator-(const vec2_t& other) const { return { x - other.x, y - other.y }; }
        constexpr vec2_t operator*(f32 scalar)          const { return { x * scalar, y * scalar }; }
        constexpr vec2_t operator/(f32 scalar)          const { return { x / scalar, y / scalar }; }

        constexpr vec2_t& operator+=(const vec2_t& other) { x += other.x; y += other.y; return *this; }
        constexpr vec2_t& operator-=(const vec2_t& other) { x -= other.x; y -= other.y; return *this; }
        constexpr vec2_t& operator*=(f32 scalar)          { x *= scalar; y *= scalar; return *this; }
        constexpr vec2_t& operator/=(f32 scalar)          { x /= scalar; y /= scalar; return *this; }

        constexpr bool   operator==(const vec2_t& other) const { return x == other.x && y == other.y; }

        f32 length() const { return std::sqrt(length_squared()); }

        constexpr f32 length_squared() const { return (x*x + y*y); }

        constexpr f32 dot(const vec2_t& other) const { return x*other.x + y*other.y; }

        vec2_t normalized() const
        {
            const f32 vector_length = length();

            if (vector_length == 0.0f)
            {
                return zero();
            }

            return *this / vector_length;
        }

        f32 distance(const vec2_t& other) const
        {
            return (*this - other).length();
        }

        constexpr f32 distance_squared(const vec2_t& other) const
        {
            return (*this - other).length_squared();
        }

        constexpr static vec2_t zero()  { return {  0,  0 }; }
        constexpr static vec2_t one()   { return {  1,  1 }; }
        constexpr static vec2_t up()    { return {  0,  1 }; }
        constexpr static vec2_t down()  { return {  0, -1 }; }
        constexpr static vec2_t left()  { return { -1,  0 }; }
        constexpr static vec2_t right() { return {  1,  0 }; }
    };

    constexpr vec2_t operator*(f32 scalar, const vec2_t& vector) { return vector * scalar; }
}

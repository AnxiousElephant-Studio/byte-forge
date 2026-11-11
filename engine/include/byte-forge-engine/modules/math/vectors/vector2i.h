#pragma once

#include <byte-forge-engine/api/types.h>

namespace byteforge::math
{
    struct vec2i_t
    {
    public:
        i32 x, y;

        constexpr vec2i_t()             : x(0), y(0) {}
        constexpr vec2i_t(i32 x, i32 y) : x(x), y(y) {}

        constexpr i32 width()  const { return x; }
        constexpr i32 height() const { return y; }

        constexpr i32  operator[](usize index) const { return index == 0 ? x : y; }
        constexpr i32& operator[](usize index)       { return index == 0 ? x : y; }

        constexpr vec2i_t operator+(const vec2i_t& other) const { return { x + other.x, y + other.y }; }
        constexpr vec2i_t operator-(const vec2i_t& other) const { return { x - other.x, y - other.y }; }
        constexpr vec2i_t operator*(i32 scalar)           const { return { x * scalar, y * scalar }; }

        constexpr vec2i_t& operator+=(const vec2i_t& other) { x += other.x; y += other.y; return *this; }
        constexpr vec2i_t& operator-=(const vec2i_t& other) { x -= other.x; y -= other.y; return *this; }

        constexpr bool operator==(const vec2i_t& other) const { return x == other.x && y == other.y; }
        constexpr bool operator!=(const vec2i_t& other) const { return x != other.x || y != other.y; }

        constexpr bool is_positive() const { return x > 0 && y > 0; }

        constexpr static vec2i_t zero() { return { 0, 0 }; }
        constexpr static vec2i_t one()  { return { 1, 1 }; }
    };
}

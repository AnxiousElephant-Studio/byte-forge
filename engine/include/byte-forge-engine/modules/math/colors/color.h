#pragma once

#include <byte-forge-engine/api/types.h>

namespace byteforge::math
{
    struct color_t
    {
    public:
        f32 r, g, b, a;

        constexpr color_t()                               : r(0), g(0), b(0), a(1) {}
        constexpr color_t(f32 r, f32 g, f32 b, f32 a = 1) : r(r), g(g), b(b), a(a) {}

        constexpr color_t operator+(const color_t& other) const
        {
            return {
                r + other.r,
                g + other.g,
                b + other.b,
                a + other.a
            };
        }

        constexpr color_t operator*(f32 scalar) const
        {
            return {
                r * scalar,
                g * scalar,
                b * scalar,
                a * scalar
            };
        }

        constexpr color_t& operator+=(const color_t& other)
        {
            r += other.r;
            g += other.g;
            b += other.b;
            a += other.a;
            return *this;
        }

        constexpr color_t& operator*=(f32 scalar)
        {
            r *= scalar;
            g *= scalar;
            b *= scalar;
            a *= scalar;
            return *this;
        }

        constexpr bool operator==(const color_t& other) const
        {
            return r == other.r &&
                   g == other.g &&
                   b == other.b &&
                   a == other.a;
        }

        constexpr static color_t black()       { return { 0, 0, 0, 1 }; }
        constexpr static color_t white()       { return { 1, 1, 1, 1 }; }
        constexpr static color_t red()         { return { 1, 0, 0, 1 }; }
        constexpr static color_t green()       { return { 0, 1, 0, 1 }; }
        constexpr static color_t blue()        { return { 0, 0, 1, 1 }; }
        constexpr static color_t yellow()      { return { 1, 1, 0, 1 }; }
        constexpr static color_t cyan()        { return { 0, 1, 1, 1 }; }
        constexpr static color_t magenta()     { return { 1, 0, 1, 1 }; }
        constexpr static color_t transparent() { return { 0, 0, 0, 0 }; }
    };

    constexpr color_t operator*(f32 scalar, const color_t& color)
    {
        return color * scalar;
    }
}

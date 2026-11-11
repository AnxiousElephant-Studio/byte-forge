#pragma once

#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/math/vectors/vector2i.h>

namespace byteforge::math
{
    struct recti_t
    {
    public:
        vec2i_t position;
        vec2i_t size;

        constexpr recti_t()                                             : position(), size() {}
        constexpr recti_t(const vec2i_t& position, const vec2i_t& size) : position(position), size(size) {}
        constexpr recti_t(i32 x, i32 y, i32 width, i32 height)          : position(x, y), size(width, height) {}

        constexpr static recti_t from_size(const vec2i_t& size) { return recti_t(vec2i_t::zero(), size); }

        constexpr i32 left()   const { return position.x; }
        constexpr i32 top()    const { return position.y; }
        constexpr i32 right()  const { return position.x + size.width(); }
        constexpr i32 bottom() const { return position.y + size.height(); }

        constexpr bool is_empty() const { return !size.is_positive(); }

        constexpr bool contains(const vec2i_t& point) const
        {
            return point.x >= left() && point.x < right() && point.y >= top() && point.y < bottom();
        }

        constexpr bool operator==(const recti_t& other) const { return position == other.position && size == other.size; }
    };
}

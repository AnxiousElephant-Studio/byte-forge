#pragma once

#include <byte-forge-engine/api/types.h>

namespace byteforge::ecs
{
    struct entity_t
    {
    public:
        u64 id;

        constexpr entity_t()       : id(0) {}
        constexpr entity_t(u64 id) : id(id) {}

        constexpr bool operator==(const entity_t& other) const { return id == other.id; }
    };
}

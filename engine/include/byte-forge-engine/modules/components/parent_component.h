#pragma once

#include <byte-forge-engine/modules/ecs/entity.h>

namespace byteforge::components
{
    struct parent_component_t
    {
        ecs::entity_t parent;
    };
}

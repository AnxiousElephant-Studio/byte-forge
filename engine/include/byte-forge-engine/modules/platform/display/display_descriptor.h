#pragma once

#include <byte-forge-engine/modules/graphics/e_graphic_backends.h>

namespace byteforge::platform
{
    struct display_descriptor_t
    {
    public:
        graphics::e_graphic_backends graphic_backend;

        explicit display_descriptor_t(const graphics::e_graphic_backends graphic_backend) : graphic_backend(graphic_backend) {}
    };
}

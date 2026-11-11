#pragma once

#include <byte-forge-engine/modules/rendering/rendering_api.h>
#include <byte-forge-engine/api/types.h>

namespace byteforge::rendering
{
    using opengl_function_t        = void (*)();
    using opengl_function_loader_t = opengl_function_t (*)(const char* function_name);

    struct BF_RENDERING_API opengl_functions_t
    {
    public:
        static bool load(opengl_function_loader_t function_loader);
        static bool are_loaded();
    };
}

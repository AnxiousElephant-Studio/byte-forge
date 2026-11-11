#include <byte-forge-engine/modules/rendering/opengl/functions/opengl_functions.h>

#include <glad/gl.h>

namespace byteforge::rendering
{
    namespace
    {
        bool functions_loaded = false;
    }

    bool opengl_functions_t::load(opengl_function_loader_t function_loader)
    {
        if (function_loader == nullptr)
        {
            return false;
        }

        functions_loaded = gladLoadGL(reinterpret_cast<GLADloadfunc>(function_loader)) != 0;

        return functions_loaded;
    }

    bool opengl_functions_t::are_loaded()
    {
        return functions_loaded;
    }
}

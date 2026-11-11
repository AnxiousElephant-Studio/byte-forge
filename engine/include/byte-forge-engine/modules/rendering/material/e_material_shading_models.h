#pragma once

#include <byte-forge-engine/api/types.h>

namespace byteforge::rendering
{
    enum class e_material_shading_models : u8
    {
        Unlit,
        Lit
    };

    constexpr const char* get_material_shading_model_name(const e_material_shading_models shading_model)
    {
        switch (shading_model)
        {
            case e_material_shading_models::Unlit: return "Unlit";
            case e_material_shading_models::Lit:   return "Lit";
        }

        return "Unknown";
    }
}

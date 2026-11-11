#pragma once

#include <byte-forge-engine/api/types.h>

namespace byteforge::graphics
{
    enum class e_graphic_backends : u8
    {
        None       = 0,
        OpenGL     = 1,
        DirectX11  = 2,
        DirectX12  = 3,
        Vulkan     = 4,
        WebGPU     = 5,
        Metal      = 6,
        Software   = 7
    };

    constexpr const char* get_graphic_backend_name(const e_graphic_backends graphic_backend)
    {
        switch (graphic_backend)
        {
            case e_graphic_backends::None:      return "None";
            case e_graphic_backends::OpenGL:    return "OpenGL";
            case e_graphic_backends::DirectX11: return "DirectX11";
            case e_graphic_backends::DirectX12: return "DirectX12";
            case e_graphic_backends::Vulkan:    return "Vulkan";
            case e_graphic_backends::WebGPU:    return "WebGPU";
            case e_graphic_backends::Metal:     return "Metal";
            case e_graphic_backends::Software:  return "Software";
        }

        return "Unknown";
    }
}

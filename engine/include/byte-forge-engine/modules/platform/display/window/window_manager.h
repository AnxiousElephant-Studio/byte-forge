#pragma once

#include <byte-forge-engine/modules/platform/platform_api.h>
#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/platform/display/window/window.h>
#include <byte-forge-engine/modules/platform/display/window/window_descriptor.h>
#include <byte-forge-engine/modules/platform/display/display_descriptor.h>

#include <vector>

namespace byteforge::platform
{
    class BF_PLATFORM_API window_manager_t
    {
    public:
        void dispose();

        window_t* create_window(const window_descriptor_t& descriptor, const display_descriptor_t& display_descriptor);
        void destroy_window(const u32 window_id);

        window_t* get_window(const u32 window_id);
        window_t* get_window_at(const usize index);
        usize get_window_count() const;

    private:
        std::vector<window_t*> windows;
    };
}

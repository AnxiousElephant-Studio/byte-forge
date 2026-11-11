#pragma once

#include <byte-forge-engine/modules/platform/platform_api.h>
#include <byte-forge-engine/modules/platform/display/display_descriptor.h>
#include <byte-forge-engine/modules/platform/display/window/window_manager.h>
#include <byte-forge-engine/modules/platform/display/window/window_event_callback.h>
#include <byte-forge-engine/modules/platform/input/input_event_callback.h>
#include <byte-forge-engine/modules/platform/native_event_callback.h>
#include <byte-forge-engine/modules/platform/platform_context.h>

namespace byteforge::platform
{
    class BF_PLATFORM_API platform_t
    {
    public:
        display_descriptor_t display_descriptor;
        window_manager_t window_manager;
        bool* should_close_handler;
        native_event_callback_t native_event_callback;
        platform_context_t context;

        platform_t(display_descriptor_t display_descriptor, bool* should_close_handler)
            : display_descriptor(display_descriptor)
            , window_manager()
            , should_close_handler(should_close_handler)
            , native_event_callback()
            , context()
            , initialized(false)
        {
        }

        bool initialize();
        void dispose();

        void poll_events(const input_event_callback_t& input_event_callback, const window_event_callback_t& window_event_callback);

        void request_close();

        void set_current_window(window_t& window);
        window_t* get_current_window() const;

    private:
        bool initialized;
    };
}

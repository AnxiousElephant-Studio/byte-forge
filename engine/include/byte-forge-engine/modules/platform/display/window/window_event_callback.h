#pragma once

#include <byte-forge-engine/modules/platform/display/window/window_event.h>

#include <functional>

namespace byteforge::platform
{
    struct window_event_callback_t
    {
    public:
        std::function<void(const window_event_t&)> callback;

        void operator()(const window_event_t& event) const
        {
            if (callback)
            {
                callback(event);
            }
        }
    };
}

#pragma once

#include <byte-forge-engine/modules/platform/input/input_event.h>

#include <functional>

namespace byteforge::platform
{
    struct input_event_callback_t
    {
    public:
        std::function<void(const input_event_t&)> callback;

        void operator()(const input_event_t& event) const
        {
            if (callback)
            {
                callback(event);
            }
        }
    };
}

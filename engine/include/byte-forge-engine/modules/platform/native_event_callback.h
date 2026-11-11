#pragma once

#include <functional>

namespace byteforge::platform
{
    struct native_event_callback_t
    {
    public:
        std::function<void(const void*)> callback;

        void operator()(const void* native_event) const
        {
            if (callback)
            {
                callback(native_event);
            }
        }
    };
}

#include <byte-forge-engine/modules/runtime/engine_runtime.h>

namespace byteforge::runtime
{
    bool engine_runtime_t::initialize()
    {
        if (!platform.initialize() ||
            !renderer.initialize(platform.display_descriptor.graphic_backend) ||
            !scripting_context.initialize() ||
            !audio.initialize())
        {
            dispose();
            return false;
        }

        frame_manager.reset(frame_settings);

        return true;
    }

    platform::window_t* engine_runtime_t::create_window(const platform::window_descriptor_t& descriptor)
    {
        platform::window_t* window = platform.window_manager.create_window(descriptor, platform.display_descriptor);

        if (window == nullptr)
        {
            return nullptr;
        }

        if (!renderer.attach_window(*window))
        {
            platform.window_manager.destroy_window(window->sdl_window_id);
            return nullptr;
        }

        return window;
    }

    void engine_runtime_t::destroy_window(u32 window_id)
    {
        renderer.detach_window(window_id);
        platform.window_manager.destroy_window(window_id);
    }

    void engine_runtime_t::render()
    {
        for (usize index = 0; index < platform.window_manager.get_window_count(); index++)
        {
            platform::window_t* window = platform.window_manager.get_window_at(index);

            if (!renderer.begin_frame(*window))
            {
                continue;
            }

            renderer.end_frame(*window);
        }
    }

    void engine_runtime_t::dispose()
    {
        asset_manager.dispose();
        audio.dispose();
        scripting_context.dispose();
        renderer.dispose();
        platform.dispose();
    }
}

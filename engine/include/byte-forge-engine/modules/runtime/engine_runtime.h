#pragma once

#include <byte-forge-engine/modules/runtime/runtime_api.h>
#include <byte-forge-engine/api/types.h>
#include <byte-forge-engine/modules/assets/asset_manager.h>
#include <byte-forge-engine/modules/audio/audio_engine.h>
#include <byte-forge-engine/modules/platform/platform.h>
#include <byte-forge-engine/modules/platform/display/display_descriptor.h>
#include <byte-forge-engine/modules/rendering/window/renderer.h>
#include <byte-forge-engine/modules/scripting/scripting_context.h>
#include <byte-forge-engine/modules/time/frame_manager.h>
#include <byte-forge-engine/modules/time/frame_settings.h>

namespace byteforge::runtime
{
    class BF_RUNTIME_API engine_runtime_t
    {
    public:
        bool should_close;
        time::frame_settings_t frame_settings;
        time::frame_manager_t frame_manager;
        platform::platform_t platform;
        rendering::renderer_t renderer;
        scripting::scripting_context_t scripting_context;
        audio::audio_engine_t audio;
        assets::asset_manager_t asset_manager;

        engine_runtime_t(platform::display_descriptor_t display_descriptor, time::frame_settings_t frame_settings)
            : should_close(false)
            , frame_settings(frame_settings)
            , frame_manager()
            , platform(display_descriptor, &should_close)
            , renderer()
            , scripting_context()
            , audio()
            , asset_manager()
        {
        }

        bool initialize();
        void dispose();

        platform::window_t* create_window(const platform::window_descriptor_t& descriptor);
        void destroy_window(u32 window_id);

        void render();
    };
}

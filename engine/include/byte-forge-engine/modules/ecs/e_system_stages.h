#pragma once

#include <byte-forge-engine/api/types.h>

namespace byteforge::ecs
{
    enum class e_system_stages : u8
    {
        Start = 0,
        Update,
        Render,
        Stop
    };

    constexpr const char* get_system_stage_name(const e_system_stages stage)
    {
        switch (stage)
        {
            case e_system_stages::Start:  return "Start";
            case e_system_stages::Update: return "Update";
            case e_system_stages::Render: return "Render";
            case e_system_stages::Stop:   return "Stop";
        }

        return "Unknown";
    }
}

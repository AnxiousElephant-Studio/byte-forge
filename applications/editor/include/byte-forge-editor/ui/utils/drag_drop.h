#pragma once

#include <byte-forge-engine/api/types.h>

#include <string>

namespace byteforge::editor
{
    struct drag_drop_payloads_t
    {
    public:
        static constexpr const char* asset_path = "BF_ASSET_PATH";
    };

    void begin_asset_path_drag(const std::string& path, const char* preview_label);

    bool accept_asset_path_drop(std::string& out_path);
    bool accept_asset_path_drop(char* out_buffer, const usize buffer_size);
}

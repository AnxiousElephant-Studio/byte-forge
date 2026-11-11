#pragma once

#include <byte-forge-engine/modules/logging/log_entry.h>

namespace byteforge::logging
{
    using log_sink_t = void (*)(const log_entry_t& entry);
}

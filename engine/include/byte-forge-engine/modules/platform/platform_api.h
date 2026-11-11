#pragma once

#include <byte-forge-engine/api/api.h>

#if defined(BF_PLATFORM_BUILD) || defined(BF_ENGINE_BUILD)
    #define BF_PLATFORM_API BF_EXPORT
#else
    #define BF_PLATFORM_API BF_IMPORT
#endif

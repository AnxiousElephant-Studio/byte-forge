#pragma once

#include <byte-forge-engine/api/api.h>

#if defined(BF_TIME_BUILD) || defined(BF_ENGINE_BUILD)
    #define BF_TIME_API BF_EXPORT
#else
    #define BF_TIME_API BF_IMPORT
#endif

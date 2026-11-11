#pragma once

#include <byte-forge-engine/api/api.h>

#if defined(BF_LEVEL_BUILD) || defined(BF_ENGINE_BUILD)
    #define BF_LEVEL_API BF_EXPORT
#else
    #define BF_LEVEL_API BF_IMPORT
#endif

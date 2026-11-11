#pragma once

#include <byte-forge-engine/api/api.h>

#if defined(BF_PHYSICS_BUILD) || defined(BF_ENGINE_BUILD)
    #define BF_PHYSICS_API BF_EXPORT
#else
    #define BF_PHYSICS_API BF_IMPORT
#endif

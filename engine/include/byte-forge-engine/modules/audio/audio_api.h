#pragma once

#include <byte-forge-engine/api/api.h>

#if defined(BF_AUDIO_BUILD) || defined(BF_ENGINE_BUILD)
    #define BF_AUDIO_API BF_EXPORT
#else
    #define BF_AUDIO_API BF_IMPORT
#endif

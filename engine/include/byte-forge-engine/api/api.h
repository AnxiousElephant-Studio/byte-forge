#pragma once

#if defined(BF_STATIC)
    #define BF_EXPORT
    #define BF_IMPORT
#elif defined(_WIN32)
    #define BF_EXPORT __declspec(dllexport)
    #define BF_IMPORT __declspec(dllimport)
#else
    #define BF_EXPORT __attribute__((visibility("default")))
    #define BF_IMPORT __attribute__((visibility("default")))
#endif

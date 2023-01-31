#pragma once

#include <intrin.h>

// MSVC
#define VL_PLATFORM_WIN 1
#define VL_COMPILER_MSVC 1

#define VL_DEBUG_BREAK() __debugbreak()
#define VL_PAUSE() _mm_pause()

// Defines
#define VL_INLINE inline // Compiler decides if it should be inlined
#define VL_FINLINE __forceinline

#define VL_LITTLE_ENDIAN 1
#define VL_BIG_ENDIAN 0

#define VL_ENDIANNESS VL_LITTLE_ENDIAN

// Macroses
#define VL_MIN(A, B) ((A) < (B) ? (A) : (B))
#define VL_MAX(A, B) ((A) > (B) ? (A) : (B))
#define VL_SWAP(A, B, T) { T = A; A = B; B = T; }

#define VL_BIT(N) ( 1 << ((N)-1) )

#define VL_SAFE_DELETE(X) \
    { \
        if ((X)) \
        { \
            delete (X); \
            X = nullptr; \
        } \
    }

#define VL_SAFE_DELETE_ARRAY(X) \
    { \
        if ((X)) \
        { \
            delete[] (X); \
            X = nullptr; \
        } \
    }

#pragma once

#include <intrin.h>

// MSVC
#define VLN_PLATFORM_WIN 1
#define VLN_COMPILER_MSVC 1

#define VLN_DEBUG_BREAK() __debugbreak()
#define VLN_PAUSE() _mm_pause()

// Defines
#define VLN_INLINE inline // Compiler decides if it should be inlined
#define VLN_FINLINE __forceinline

#define VLN_LITTLE_ENDIAN 1
#define VLN_BIG_ENDIAN 0

#define VLN_ENDIANNESS VLN_LITTLE_ENDIAN

// Macroses
#define VLN_MIN(A, B) ((A) < (B) ? (A) : (B))
#define VLN_MAX(A, B) ((A) > (B) ? (A) : (B))
#define VLN_SWAP(A, B, T) { T = A; A = B; B = T; }

#define VLN_BIT(N) ( 1 << ((N)-1) )

#define VLN_SAFE_DELETE(X) \
    { \
        if ((X)) \
        { \
            delete (X); \
            X = nullptr; \
        } \
    }

#define VLN_SAFE_DELETE_ARRAY(X) \
    { \
        if ((X)) \
        { \
            delete[] (X); \
            X = nullptr; \
        } \
    }

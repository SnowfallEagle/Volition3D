#pragma once

#include <intrin.h>

// MSVC
#define VL_PLATFORM_WIN 1
#define VL_COMPILER_MSVC 1

#define DEBUG_BREAK() __debugbreak()
#define PAUSE() _mm_pause()

// Defines
#define INLINE inline // Compiler decides if it should be inlined
#define FINLINE __forceinline

#define VL_LITTLE_ENDIAN 1
#define VL_BIG_ENDIAN 0

#define VL_ENDIANNESS VL_LITTLE_ENDIAN

// Macroses
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define SWAP(A, B, T) { T = A; A = B; B = T; }

#define BIT(N) ( 1 << ((N)-1) )

#define SAFE_DELETE(X) \
    { \
        if ((X)) \
        { \
            delete (X); \
            X = nullptr; \
        } \
    }

#define SAFE_DELETE_ARRAY(X) \
    { \
        if ((X)) \
        { \
            delete[] (X); \
            X = nullptr; \
        } \
    }

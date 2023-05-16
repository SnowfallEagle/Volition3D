#pragma once

#include <intrin.h>
#include <xmmintrin.h>

namespace Volition
{

#define VLN_PLATFORM_WIN 1
#define VLN_COMPILER_MSVC 1

#define VLN_INLINE inline // Compiler decides if it should be inlined
#define VLN_FINLINE __forceinline
#define VLN_NINLINE __declspec(noinline)

#define VLN_LITTLE_ENDIAN 1
#define VLN_BIG_ENDIAN 0
#define VLN_ENDIANNESS VLN_LITTLE_ENDIAN

#define VLN_SSE 1

#define VLN_DEBUG_BREAK() __debugbreak()
#define VLN_PAUSE() _mm_pause()

#define VLN_DECL_ALIGN(N) __declspec(align(N))
#define VLN_DEFINE_ALIGN_OPERATORS(N) \
    VLN_FINLINE void* operator new(size_t Size) \
    { \
        return _mm_malloc(Size, N); \
    } \
 \
    VLN_FINLINE void* operator new[](size_t Size) \
    { \
        return _mm_malloc(Size, N); \
    } \
 \
    VLN_FINLINE void operator delete(void* Ptr) \
    { \
        _mm_free(Ptr); \
    } \
 \
    VLN_FINLINE void operator delete[](void* Ptr) \
    { \
        _mm_free(Ptr); \
    } \

#define VLN_DECL_ALIGN_SSE() VLN_DECL_ALIGN(16)
#define VLN_DEFINE_ALIGN_OPERATORS_SSE() VLN_DEFINE_ALIGN_OPERATORS(16)

#define VLN_MIN(A, B) ((A) < (B) ? (A) : (B))
#define VLN_MAX(A, B) ((A) > (B) ? (A) : (B))
#define VLN_SWAP(A, B, T) { T = A; A = B; B = T; }

#define VLN_BIT(N) ( 1 << ((N)-1) )

#define VLN_ARRAY_SIZE(A) (sizeof(A) / sizeof(A[0]))

#define VLN_SAFE_DELETE(X) \
    { \
        if ((X)) \
        { \
            delete (X); \
            (X) = nullptr; \
        } \
    }

#define VLN_SAFE_DELETE_ARRAY(X) \
    { \
        if ((X)) \
        { \
            delete[] (X); \
            (X) = nullptr; \
        } \
    }

}

#ifndef ASSERT_H_

// === Debug break ===
// TODO(sean): Make platform header
#include <intrin.h>
#define VL_DEBUG_BREAK() __debugbreak()

// === Run-time assert ===
#if defined(_DEBUG) || defined(DEBUG)
# define VL_ASSERT(EXPR) \
    if (EXPR) \
    {} \
    else \
    { \
        fprintf(stderr, "Assertion failed at %s:%d: %s\n", __FILE__, __LINE__, #EXPR); \
        VL_DEBUG_BREAK(); \
    }
#else
# define VL_DEBUG_BREAK()
# define VL_ASSERT(EXPR)
#endif

// === Static assert ===
#define _VL_GLUE_STATIC_ASSERT(A, B) A ## B
#define VL_GLUE_STATIC_ASSERT(A, B) _VL_GLUE_STATIC_ASSERT(A, B)

#if __cplusplus >= 201103L || defined(_MSC_VER)
# define VL_STATIC_ASSERT(EXPR) \
    static_assert(EXPR, "Static assertion failed: " #EXPR)
#else
    template<bool> class TStaticAssert;
    template<> class TStaticAssert<true> {};

# define VL_STATIC_ASSERT(EXPR) \
    enum { VL_GLUE_STATIC_ASSERT(ASSERT_FAIL_, __LINE__) = sizeof(TStaticAssert<!!(EXPR)>) }
#endif

#define ASSERT_H_
#endif

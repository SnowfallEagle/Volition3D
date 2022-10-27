#ifndef ASSERT_H_

// === Debug break ===
// TODO(sean): Make platform header
#ifdef _MSC_VER
# include <intrin.h>
# define DEBUG_BREAK() __debugbreak()
# define PAUSE() _mm_pause()
#endif

// === Run-time assert ===
#if defined(_DEBUG) || defined(DEBUG)
# define ASSERT(EXPR) \
    if (EXPR) \
    {} \
    else \
    { \
        fprintf(stderr, "Assertion failed at %s:%d: %s\n", __FILE__, __LINE__, #EXPR); \
        DEBUG_BREAK(); \
    }
#else
# define DEBUG_BREAK()
# define ASSERT(EXPR)
#endif

// === Static assert ===
#define _GLUE_STATIC_ASSERT(A, B) A ## B
#define GLUE_STATIC_ASSERT(A, B) _GLUE_STATIC_ASSERT(A, B)

#if __cplusplus >= 201103L || defined(_MSC_VER)
# define STATIC_ASSERT(EXPR) \
    static_assert(EXPR, "Static assertion failed: " #EXPR)
#else
    template<bool> class TStaticAssert;
    template<> class TStaticAssert<true> {};

# define STATIC_ASSERT(EXPR) \
    enum { GLUE_STATIC_ASSERT(ASSERT_FAIL_, __LINE__) = sizeof(TStaticAssert<!!(EXPR)>) }
#endif

#define ASSERT_H_
#endif

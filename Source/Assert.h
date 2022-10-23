#ifndef ASSERT_H_

// === Run-time assert ===
// TODO(sean): Try some assembly stuff instead of stdlib's exit()
#if defined(_DEBUG) || defined(DEBUG)
# include <stdlib.h>
# define DEBUG_BREAK() { exit(1); }

# define ASSERT(EXPR) \
    if (EXPR) \
    {} \
    else \
    { \
        fprintf(stderr, "Assertion failed at %s:%d: %s\n", __FILE__, __LINE__, #EXPR); \
        DEBUG_BREAK(); \
    }
#else
# define ASSERT(EXPR)
#endif

// === Static assert ===
#define _GLUE_STATIC_ASSERT(A, B) A ## B
#define GLUE_STATIC_ASSERT(A, B) _GLUE_STATIC_ASSERT(A, B)

#if __cplusplus >= 201103L || defined(_MSC_VER)
# define STATIC_ASSERT(EXPR) \
    static_assert(EXPR, \
                  "Static assertion failed at " \
                  __FILE__ ":" \
                  STATIC_ASSERT_TO_STRING(__LINE__) ": " \
                  #EXPR)
#else
    template<bool> class TStaticAssert;
    template<> class TStaticAssert<true> {};

# define STATIC_ASSERT(EXPR) \
    enum { GLUE_STATIC_ASSERT(ASSERT_FAIL_, __LINE__) = sizeof(TStaticAssert<!!(EXPR)>) }
#endif

#define ASSERT_H_
#endif

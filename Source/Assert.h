#ifndef ASSERT_H_

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

#define ASSERT_H_
#endif

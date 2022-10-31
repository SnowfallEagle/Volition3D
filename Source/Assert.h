#ifndef ASSERT_H_

// === Platform stuff ===
// TODO(sean): Make platform header
#ifdef _MSC_VER
# include <intrin.h>
# define DEBUG_BREAK() __debugbreak()
# define PAUSE() _mm_pause()
#else
# define DEBUG_BREAK()
# define PAUSE()
#endif

// === Debug assertions ===
#ifdef _DEBUG

# include <stdio.h> // TODO(sean): Make my log system

// Assert
# define ASSERT(EXPR) \
    if ((EXPR)) \
    {} \
    else \
    { \
        fprintf(stderr, "Assertion failed at %s:%d: %s\n", __FILE__, __LINE__, #EXPR); \
        DEBUG_BREAK(); \
    }

// Static assert
# define _GLUE_STATIC_ASSERT(A, B) A ## B
# define GLUE_STATIC_ASSERT(A, B) _GLUE_STATIC_ASSERT(A, B)

# if __cplusplus >= 201103L || defined(_MSC_VER)
#  define STATIC_ASSERT(EXPR) \
    static_assert(EXPR, "Static assertion failed: " #EXPR)
# else
    template<bool> class TStaticAssert;
    template<> class TStaticAssert<true> {};
#  define STATIC_ASSERT(EXPR) \
    enum { GLUE_STATIC_ASSERT(ASSERT_FAIL_, __LINE__) = sizeof(TStaticAssert<!!(EXPR)>) }
# endif

// Unnecessary lock assert
    class VUnnecessaryLock
    {
        volatile bool bLocked = false;

    public:
        void Acquire()
        {
            ASSERT(!bLocked);
            bLocked = true;
        }

        void Release()
        {
            ASSERT(bLocked);
            bLocked = false;
        }
    };

    class VUnnecessaryLockJanitor
    {
        VUnnecessaryLock* Lock;

    public:
        VUnnecessaryLockJanitor(VUnnecessaryLock& InLock) : Lock(&InLock)
        {
            Lock->Acquire();
        }
        ~VUnnecessaryLockJanitor()
        {
            Lock->Release();
        }
    };

# define BEGIN_ASSERT_LOCK_NOT_NECESSARY(L) (L).Acquire()
# define END_ASSERT_LOCK_NOT_NECESSARY(L) (L).Release()
# define ASSERT_LOCK_NOT_NECESSARY(J, L) VUnnecessaryLockJanitor J(L)

#else

# define ASSERT(EXPR)
# define STATIC_ASSERT(EXPR)
# define BEGIN_ASSERT_LOCK_NOT_NECESSARY(L)
# define END_ASSERT_LOCK_NOT_NECESSARY(L)
# define ASSERT_LOCK_NOT_NECESSARY(J, L)

#endif // _DEBUG

#define ASSERT_H_
#endif

#ifndef CORE_ASSERT_H_

#include "Core/Platform.h"
#include "Core/DebugLog.h"

DEFINE_LOG_CHANNEL(hLogAssert, "Assert");

#define ASSERTIONS_ENABLED 1
#if ASSERTIONS_ENABLED

// Assert
# define ASSERT(EXPR) \
    if ((EXPR)) \
    {} \
    else \
    { \
        VL_ERROR(hLogAssert, "Assertion failed at %s:%d: %s\n", __FILE__, __LINE__, #EXPR); \
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

#define CORE_ASSERT_H_
#endif

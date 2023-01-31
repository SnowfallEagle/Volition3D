#pragma once

#include "Engine/Core/Platform.h"
#include "Engine/Core/DebugLog.h"

VL_DEFINE_LOG_CHANNEL(hLogAssert, "Assert");

#define VL_ASSERTIONS_ENABLED 1

#if VL_ASSERTIONS_ENABLED
    // Assert
    #define VL_ASSERT(EXPR) \
        if ((EXPR)) \
        {} \
        else \
        { \
            VL_ERROR(hLogAssert, "Assertion failed at %s:%d: %s\n", __FILE__, __LINE__, #EXPR); \
            VL_DEBUG_BREAK(); \
        }

    // Static assert
    #define _VL_GLUE_STATIC_ASSERT(A, B) A ## B
    #define VL_GLUE_STATIC_ASSERT(A, B) _VL_GLUE_STATIC_ASSERT(A, B)

    #if __cplusplus >= 201103L || defined(_MSC_VER)
        #define VL_STATIC_ASSERT(EXPR) static_assert(EXPR, "Static assertion failed: " #EXPR)
    #else
        template<bool> class TStaticAssert;
        template<> class TStaticAssert<true> {};
        #define VL_STATIC_ASSERT(EXPR) enum { VL_GLUE_STATIC_ASSERT(ASSERT_FAIL_, __LINE__) = sizeof(TStaticAssert<!!(EXPR)>) }
    #endif

    // Unnecessary lock assert
    class VUnnecessaryLock
    {
        volatile bool bLocked = false;

    public:
        void Acquire()
        {
            VL_ASSERT(!bLocked);
            bLocked = true;
        }

        void Release()
        {
            VL_ASSERT(bLocked);
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

    #define VL_BEGIN_ASSERT_LOCK_NOT_NECESSARY(L) (L).Acquire()
    #define VL_END_ASSERT_LOCK_NOT_NECESSARY(L) (L).Release()
    #define VL_ASSERT_LOCK_NOT_NECESSARY(J, L) VUnnecessaryLockJanitor J(L)
#else
    #define VL_ASSERT(EXPR)
    #define VL_STATIC_ASSERT(EXPR)
    #define VL_BEGIN_ASSERT_LOCK_NOT_NECESSARY(L)
    #define VL_END_ASSERT_LOCK_NOT_NECESSARY(L)
    #define VL_ASSERT_LOCK_NOT_NECESSARY(J, L)
#endif // VL_ASSERTIONS_ENABLED


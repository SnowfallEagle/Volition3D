#pragma once

#ifndef VLN_ASSERTIONS_ENABLED
    #define VLN_ASSERTIONS_ENABLED 1
#endif

#ifndef VLN_ASSERTIONS_LOG_ENGINE
    #define VLN_ASSERTIONS_ENGINE_LOGGING 1
#endif

#include "Engine/Core/Platform.h"

#if VLN_ASSERTIONS_ENGINE_LOGGING
    #include "Engine/Core/DebugLog.h"

    VLN_DEFINE_LOG_CHANNEL(hLogAssert, "Assert");
#else
    #include <cstdio>
#endif

namespace Volition
{
#if VLN_ASSERTIONS_ENABLED
    #if VLN_ASSERTIONS_ENGINE_LOGGING
        #define VLN_ASSERTION_LOG_ERROR(EXPR) VLN_ERROR(hLogAssert, "Assertion failed at %s:%d: %s\n", __FILE__, __LINE__, #EXPR)
    #else
        #define VLN_ASSERTION_LOG_ERROR(EXPR) std::fprintf(stderr, "<Assert> Assertion failed at %s:%d: %s\n", __FILE__, __LINE__, #EXPR);
    #endif

    // Assert
    #define VLN_ASSERT(EXPR) \
        if ((EXPR)) \
        {} \
        else \
        { \
            VLN_ASSERTION_LOG_ERROR(EXPR); \
            VLN_DEBUG_BREAK(); \
        }

    // Static assert
    #define _VLN_GLUE_STATIC_ASSERT(A, B) A ## B
    #define VLN_GLUE_STATIC_ASSERT(A, B) _VLN_GLUE_STATIC_ASSERT(A, B)

    #if __cplusplus >= 201103L || defined(_MSC_VER)
        #define VLN_STATIC_ASSERT(EXPR) static_assert(EXPR, "Static assertion failed: " #EXPR)
    #else
        template<bool> class TStaticAssert;
        template<> class TStaticAssert<true> {};
        #define VLN_STATIC_ASSERT(EXPR) enum { VLN_GLUE_STATIC_ASSERT(ASSERT_FAIL_, __LINE__) = sizeof(TStaticAssert<!!(EXPR)>) }
    #endif

    // Unnecessary lock assert
    class VUnnecessaryLock
    {
        volatile bool bLocked = false;

    public:
        void Acquire()
        {
            VLN_ASSERT(!bLocked);
            bLocked = true;
        }

        void Release()
        {
            VLN_ASSERT(bLocked);
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

    #define VLN_BEGIN_ASSERT_LOCK_NOT_NECESSARY(L) (L).Acquire()
    #define VLN_END_ASSERT_LOCK_NOT_NECESSARY(L) (L).Release()
    #define VLN_ASSERT_LOCK_NOT_NECESSARY(J, L) VUnnecessaryLockJanitor J(L)
#else
    #define VLN_ASSERT(EXPR)
    #define VLN_STATIC_ASSERT(EXPR)
    #define VLN_BEGIN_ASSERT_LOCK_NOT_NECESSARY(L)
    #define VLN_END_ASSERT_LOCK_NOT_NECESSARY(L)
    #define VLN_ASSERT_LOCK_NOT_NECESSARY(J, L)
#endif // VLN_ASSERTIONS_ENABLED

}

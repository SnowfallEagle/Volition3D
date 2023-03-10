#pragma once

#include <atomic>
#include <thread>
#include "Engine/Core/Types.h"
#include "Engine/Core/Platform.h"
#include "Engine/Core/Assert.h"

namespace Volition
{

class VSpinLock
{
    std::atomic_flag Atomic;

public:
    VSpinLock()
    {
        Atomic.clear();
    }

    b32 TryAcquire()
    {
        // All subsequent reads will be valid
        bool bAlreadyLocked = Atomic.test_and_set(std::memory_order_acquire);
        return !bAlreadyLocked;
    }

    void Acquire()
    {
        while (!TryAcquire())
        {
            VLN_PAUSE();
        }
    }

    void Release()
    {
        // Ensure we wrote everything before we clear flag
        Atomic.clear(std::memory_order_release);
    }
};

template<class LockT>
class TScopedLock
{
public:
    using VLockType = LockT;

private:
    VLockType* Lock;

public:
    TScopedLock(VLockType& InLock) : Lock(&InLock)
    {
        Lock->Acquire();
    }

    ~TScopedLock()
    {
        Lock->Release();
    }
};

class VReentrantLock32
{
    std::atomic<std::size_t> Owner;
    i32 RefCount;

public:
    VReentrantLock32() : Owner(0), RefCount(0) {}

    b32 TryAcquire()
    {
        std::hash<std::thread::id> Hasher;
        std::size_t ThreadID = Hasher(std::this_thread::get_id());

        b32 bAcquired;

        if (ThreadID == Owner.load(std::memory_order_relaxed))
        {
            bAcquired = true;
        }
        else
        {
            std::size_t UnlockValue = 0;
            bAcquired = Owner.compare_exchange_strong(
                UnlockValue, ThreadID,
                std::memory_order_relaxed, // Fence below
                std::memory_order_relaxed
            );
        }

        if (bAcquired)
        {
            ++RefCount;
            std::atomic_thread_fence(std::memory_order_acquire);
        }

        return bAcquired;
    }

    void Acquire()
    {
        std::hash<std::thread::id> Hasher;
        std::size_t ThreadID = Hasher(std::this_thread::get_id());

        if (ThreadID != Owner.load(std::memory_order_relaxed))
        {
            std::size_t UnlockValue = 0;
            while (!Owner.compare_exchange_weak(
                UnlockValue, ThreadID,
                std::memory_order_relaxed, // Fence below
                std::memory_order_relaxed))
            {
                UnlockValue = 0;
                VLN_PAUSE();
            }
        }

        ++RefCount;
        std::atomic_thread_fence(std::memory_order_acquire);
    }

    void Release()
    {
        // Ensure that we got all writes before we unlock
        std::atomic_thread_fence(std::memory_order_release);

        std::hash<std::thread::id> Hasher;
        std::size_t ThreadID = Hasher(std::this_thread::get_id());
        std::size_t ActualID = Owner.load(std::memory_order_relaxed);
        VLN_ASSERT(ThreadID == ActualID);

        --RefCount;
        if (RefCount == 0)
        {
            Owner.store(0, std::memory_order_relaxed);
        }
    }
};

class VPushLock
{
    std::atomic<u32> RefCount;

public:
    VPushLock() : RefCount(0) {}

    void AcquireRead()
    {
        u32 CurrentRefCount = RefCount.load(std::memory_order_relaxed);
        while (!RefCount.compare_exchange_weak(
            CurrentRefCount, CurrentRefCount + 1,
            std::memory_order_acquire,
            std::memory_order_relaxed))
        {
            VLN_PAUSE();
            std::size_t CurrentRefCount = RefCount.load(std::memory_order_relaxed);
        }
    }

    void ReleaseRead()
    {
        u32 CurrentRefCount = RefCount.load(std::memory_order_relaxed);
        while (!RefCount.compare_exchange_weak(
            CurrentRefCount, CurrentRefCount - 1,
            std::memory_order_acquire,
            std::memory_order_relaxed))
        {
            VLN_PAUSE();
            std::size_t CurrentRefCount = RefCount.load(std::memory_order_relaxed);
        }

        VLN_ASSERT(CurrentRefCount > 0);
    }

    void AcquireWrite()
    {
        u32 UnlockValue = 0;
        while (!RefCount.compare_exchange_weak(
               UnlockValue, 0xFFFFFFFF,
               std::memory_order_acquire,
               std::memory_order_relaxed))
        {
            VLN_PAUSE();
            UnlockValue = 0;
        }
    }

    void ReleaseWrite()
    {
        VLN_ASSERT(0xFFFFFFFF == RefCount.load(std::memory_order_relaxed))
        RefCount.store(0, std::memory_order_release);
    }
};

}

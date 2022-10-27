#ifndef SPINLOCK_H_

#include <atomic>
#include <thread> // TODO(sean): Make volition threads
#include "Types.h"

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
            PAUSE();
    }

    void Release()
    {
        // Ensure we wrote everything before we clear flag
        Atomic.clear(std::memory_order_release);
    }
};

template <class LOCK>
class TScopedLock
{
    typedef LOCK VLockType;
    LOCK* Lock;

public:
    TScopedLock(LOCK& InLock) : Lock(&InLock)
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
            bAcquired = Owner.compare_exchange_strong(UnlockValue,
                                                      ThreadID,
                                                      std::memory_order_relaxed, // Fence below
                                                      std::memory_order_relaxed
            );
        }

        if (bAcquired)
            ++RefCount;
        std::atomic_thread_fence(std::memory_order_acquire);

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
                UnlockValue,
                ThreadID,
                std::memory_order_relaxed, // Fence below
                std::memory_order_relaxed
            ))
            {
                UnlockValue = 0;
                PAUSE();
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
        ASSERT(ThreadID == ActualID);

        --RefCount;
        if (RefCount <= 0)
        {
            // It's safe to unlock relaxed because we own it
            Owner.store(0, std::memory_order_relaxed);
        }
    }
};

#define SPINLOCK_H_
#endif

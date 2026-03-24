/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base/macros.h>

#include <atomic>
#include <semaphore>
#include <mutex>

namespace cflib::base {

class Mutex
{
    CF_DISABLE_COPY(Mutex)
public:
    Mutex() = default;

    void lock()   { m_.lock();   }
    void unlock() { m_.unlock(); }

    std::mutex & native() { return m_; }

private:
    std::mutex m_;
};

class MutexLocker
{
    CF_DISABLE_COPY(MutexLocker)
public:
    explicit MutexLocker(Mutex & m) : lock_(m.native()) {}

private:
    std::unique_lock<std::mutex> lock_;
};

class Semaphore
{
    CF_DISABLE_COPY(Semaphore)
public:
    Semaphore(size_t initial = 0) : sem_(initial) {}

    void acquire() { sem_.acquire(); }
    void acquire(size_t n) { while (n-- > 0) sem_.acquire(); }
    void release(size_t n = 1) { sem_.release(n); }

private:
    std::binary_semaphore sem_;
};

template<typename T>
class AtomicInteger
{
    CF_DISABLE_COPY(AtomicInteger)
public:
    AtomicInteger(T v = 0) : val_(v) {}

    T loadRelaxed() const { return val_.load(std::memory_order_relaxed); }
    void storeRelaxed(T v) { val_.store(v, std::memory_order_relaxed); }
    void storeRelease(T v) { val_.store(v, std::memory_order_release); }

    bool testAndSetAcquire(T expected, T newVal) {
        return val_.compare_exchange_weak(expected, newVal, std::memory_order_acquire, std::memory_order_relaxed);
    }

    T fetchAndAddRelaxed(T v) { return val_.fetch_add(v, std::memory_order_relaxed); }

    T operator++() { return val_.fetch_add(1, std::memory_order_acq_rel) + 1; }
    T operator++(int) { return val_.fetch_add(1, std::memory_order_acq_rel); }

    bool ref()   { return val_.fetch_add(1, std::memory_order_acq_rel) + 1 != 0; }
    bool deref() { return val_.fetch_sub(1, std::memory_order_acq_rel) - 1 != 0; }

    static inline void yieldCPU()
    {
        #if defined(__i386__) || defined(__x86_64__)
            #if defined(_MSC_VER)
                _mm_pause();
            #else
                __builtin_ia32_pause();
            #endif
        #elif defined(__arm__) || defined(__aarch64__)
            asm volatile("yield" ::: "memory");
        #endif
    }

    std::atomic<T> & native() { return val_; }

private:
    std::atomic<T> val_;
};

using AtomicInt = AtomicInteger<ssize_t>;

} // namespace

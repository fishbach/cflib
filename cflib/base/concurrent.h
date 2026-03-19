/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base/macros.h>

#include <atomic>
#include <condition_variable>
#include <mutex>

namespace cflib::base {

class Mutex
{
    CF_DISABLE_COPY(Mutex)
public:
    Mutex() = default;
    void lock()   { m_.lock(); }
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
    Semaphore(int initial = 0) : count_(initial) {}

    void acquire(int n = 1) {
        for (int i = 0; i < n; ++i) {
            std::unique_lock<std::mutex> lock(m_);
            cv_.wait(lock, [this]() { return count_ > 0; });
            --count_;
        }
    }

    void release(int n = 1) {
        {
            std::lock_guard<std::mutex> lock(m_);
            count_ += n;
        }
        if (n == 1) cv_.notify_one();
        else        cv_.notify_all();
    }

private:
    std::mutex m_;
    std::condition_variable cv_;
    int count_;
};

class AtomicInt
{
public:
    AtomicInt(int v = 0) : val_(v) {}

    int load() const { return val_.load(std::memory_order_relaxed); }
    void store(int v) { val_.store(v, std::memory_order_relaxed); }

    bool testAndSetAcquire(int expected, int newVal) {
        return val_.compare_exchange_strong(expected, newVal, std::memory_order_acquire, std::memory_order_relaxed);
    }
    void storeRelease(int v) { val_.store(v, std::memory_order_release); }

    int fetchAndAddRelaxed(int v) { return val_.fetch_add(v, std::memory_order_relaxed); }

    // ref() returns true if new value != 0
    bool ref() { return val_.fetch_add(1, std::memory_order_acq_rel) + 1 != 0; }
    // deref() returns false if new value == 0
    bool deref() { return val_.fetch_sub(1, std::memory_order_acq_rel) - 1 != 0; }

    std::atomic<int> & native() { return val_; }
private:
    std::atomic<int> val_;
};

template<typename T>
class AtomicInteger
{
public:
    AtomicInteger(T v = 0) : val_(v) {}
    T load() const { return val_.load(std::memory_order_relaxed); }
    void store(T v) { val_.store(v, std::memory_order_relaxed); }
    T operator++() { return val_.fetch_add(1, std::memory_order_relaxed) + 1; }
    T operator++(int) { return val_.fetch_add(1, std::memory_order_relaxed); }
    operator T() const { return val_.load(std::memory_order_relaxed); }
private:
    std::atomic<T> val_;
};

} // namespace

/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/base.h>
#include <cflib/util/test.h>

#include <chrono>
#include <format>
#include <iostream>
#include <thread>
#include <vector>

using namespace cflib::base;

namespace {

const size_t NUM_THREADS = 8;
const size_t OPERATIONS_PER_THREAD = 6000000;

}

TEST_SUITE("Concurrent") {

TEST_CASE("Concurrent: atomicInt_increment_race")
{
    AtomicInteger<int> counter(0);
    std::vector<std::thread> threads;
    threads.reserve(NUM_THREADS);

    auto worker = [&counter]() {
        for (size_t i = 0; i < OPERATIONS_PER_THREAD; ++i) {
            counter.fetchAndAddRelaxed(1);
        }
    };

    auto start = std::chrono::high_resolution_clock::now();
    for (size_t t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back(worker);
    }
    for (auto & th : threads) {
        th.join();
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    size_t expected = NUM_THREADS * OPERATIONS_PER_THREAD;
    REQUIRE_EQ(counter.loadAcquire(), (int)expected);
    std::cout << std::format("  duration: {} ms\n", duration);
}

TEST_CASE("Concurrent: atomicInt_cas_race")
{
    AtomicInteger<int> value(0);
    std::vector<std::thread> threads;
    threads.reserve(NUM_THREADS);

    std::atomic<size_t> successCount = 0;

    auto worker = [&value, &successCount]() {
        for (size_t i = 0; i < OPERATIONS_PER_THREAD; ++i) {
            int expected = value.loadAcquire();
            if (value.testAndSetAcquire(expected, expected + 1)) {
                successCount.fetch_add(1, std::memory_order_relaxed);
            }
        }
    };

    auto start = std::chrono::high_resolution_clock::now();
    for (size_t t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back(worker);
    }
    for (auto & th : threads) {
        th.join();
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    REQUIRE_EQ(value.loadAcquire(), (int)successCount.load(std::memory_order_relaxed));
    REQUIRE(value.loadAcquire() <= (int)(NUM_THREADS * OPERATIONS_PER_THREAD));
    std::cout << std::format("  duration: {} ms, success: {}\n", duration, successCount.load(std::memory_order_relaxed));
}

TEST_CASE("Concurrent: atomicInt_ref_deref_race")
{
    AtomicInteger<int> refCount(1);
    std::vector<std::thread> threads;
    threads.reserve(NUM_THREADS);

    auto refWorker = [&refCount]() {
        for (size_t i = 0; i < OPERATIONS_PER_THREAD / 2; ++i) {
            refCount.ref();
            refCount.deref();
        }
    };

    auto derefWorker = [&refCount]() {
        for (size_t i = 0; i < OPERATIONS_PER_THREAD / 2; ++i) {
            refCount.deref();
            refCount.ref();
        }
    };

    auto start = std::chrono::high_resolution_clock::now();
    for (size_t t = 0; t < NUM_THREADS / 2; ++t) {
        threads.emplace_back(refWorker);
    }
    for (size_t t = 0; t < NUM_THREADS / 2; ++t) {
        threads.emplace_back(derefWorker);
    }
    for (auto & th : threads) {
        th.join();
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    REQUIRE(refCount.loadAcquire() > 0);
    std::cout << std::format("  duration: {} ms, final ref: {}\n", duration, refCount.loadAcquire());
}

TEST_CASE("Concurrent: atomicBool_toggle_race")
{
    AtomicBool flag(false);
    std::vector<std::thread> threads;
    threads.reserve(NUM_THREADS);

    std::atomic<size_t> flipCount = 0;

    auto worker = [&flag, &flipCount]() {
        for (size_t i = 0; i < OPERATIONS_PER_THREAD / 2; ++i) {
            bool old = flag.loadAcquire();
            flag.storeRelease(!old);
            flipCount.fetch_add(1, std::memory_order_relaxed);
        }
    };

    auto start = std::chrono::high_resolution_clock::now();
    for (size_t t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back(worker);
    }
    for (auto & th : threads) {
        th.join();
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    REQUIRE((!flag.loadAcquire() || flag.loadAcquire()));
    REQUIRE_EQ(flipCount.load(std::memory_order_relaxed), NUM_THREADS * OPERATIONS_PER_THREAD / 2);
    std::cout << std::format("  duration: {} ms, final flag: {}\n", duration, flag.loadAcquire());
}

TEST_CASE("Concurrent: mutex_lock_race")
{
    Mutex mutex;
    int sharedCounter = 0;
    std::vector<std::thread> threads;
    threads.reserve(NUM_THREADS);

    auto worker = [&mutex, &sharedCounter]() {
        for (size_t i = 0; i < OPERATIONS_PER_THREAD; ++i) {
            MutexLocker lock(mutex);
            ++sharedCounter;
        }
    };

    auto start = std::chrono::high_resolution_clock::now();
    for (size_t t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back(worker);
    }
    for (auto & th : threads) {
        th.join();
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    size_t expected = NUM_THREADS * OPERATIONS_PER_THREAD;
    REQUIRE_EQ(sharedCounter, (int)expected);
    std::cout << std::format("  duration: {} ms\n", duration);
}

TEST_CASE("Concurrent: semaphore_producer_consumer")
{
    const size_t ITEM_COUNT = 50000;
    Semaphore sem(0);
    std::vector<int> buffer;
    Mutex bufferMutex;
    std::vector<std::thread> threads;
    threads.reserve(2);

    auto producer = [&sem, &buffer, &bufferMutex]() {
        for (size_t i = 0; i < ITEM_COUNT; ++i) {
            {
                MutexLocker lock(bufferMutex);
                buffer.push_back((int)i);
            }
            sem.release();
        }
    };

    auto consumer = [&sem, &buffer, &bufferMutex]() {
        for (size_t i = 0; i < ITEM_COUNT; ++i) {
            sem.acquire();
            {
                MutexLocker lock(bufferMutex);
                if (!buffer.empty()) {
                    buffer.pop_back();
                }
            }
        }
    };

    auto start = std::chrono::high_resolution_clock::now();
    threads.emplace_back(producer);
    threads.emplace_back(consumer);
    for (auto & th : threads) {
        th.join();
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    REQUIRE(buffer.empty());
    std::cout << std::format("  duration: {} ms\n", duration);
}

TEST_CASE("Concurrent: mixed_atomic_operations")
{
    AtomicInteger<int> counter(0);
    AtomicBool ready(false);
    std::vector<std::thread> threads;
    threads.reserve(NUM_THREADS);

    auto worker = [&counter, &ready]() {
        while (!ready.loadAcquire()) {
            AtomicInteger<int>::yieldCPU();
        }
        for (size_t i = 0; i < OPERATIONS_PER_THREAD; ++i) {
            counter.fetchAndAddRelaxed(1);
            AtomicInteger<int>::yieldCPU();
        }
    };

    auto start = std::chrono::high_resolution_clock::now();
    for (size_t t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back(worker);
    }
    ready.storeRelease(true);
    for (auto & th : threads) {
        th.join();
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    size_t expected = NUM_THREADS * OPERATIONS_PER_THREAD;
    REQUIRE_EQ(counter.loadAcquire(), (int)expected);
    std::cout << std::format("  duration: {} ms\n", duration);
}

// COW diverge: N threads each copy a shared source block, then grow their own
// private copy. The first append on each copy triggers detach() -> Shared::copy
// (reads the shared block) and grow() -> realloc, which may relocate the block
// and thereby the AtomicInt refcount that lives in its header. This exercises
// refcount atomicity, COW reads of the shared block, and the realloc move under
// concurrency. TSan must report no data race and every result must be correct.
TEST_CASE("Concurrent: byteArray_cow_diverge")
{
    const size_t N = 8;
    const size_t GROWS = 20000;
    const size_t CHUNK = 4;

    ByteArray source(100, 'a');           // shared block, ref == 1
    const size_t baseLen = source.size();

    std::atomic<bool> failed{false};
    std::vector<std::thread> threads;
    threads.reserve(N);

    for (size_t t = 0; t < N; ++t) {
        threads.emplace_back([&source, baseLen, GROWS, &failed, t]() {
            ByteArray local(source);      // share source's block (ref++)
            char chunk[CHUNK] = {'t', char('a' + (t % 26)), char('0' + (t % 10)), 'x'};
            for (size_t i = 0; i < GROWS; ++i) {
                local.append(chunk, CHUNK);   // detach (COW) + grow (realloc)
            }
            if (local.size() != baseLen + GROWS * CHUNK ||
                !local.startsWith("aaaa")) {
                failed.store(true);
            }
        });
    }
    for (auto & th : threads) th.join();

    REQUIRE(!failed.load());
    // Writers copied-on-write; the shared source must be untouched.
    REQUIRE_EQ(source.size(), baseLen);
    REQUIRE(source.startsWith("aaaa"));
}

// Read-mostly: N readers concurrently read a shared block (size, first/last
// byte, prefix) while a single writer diverges via COW and repeatedly grows
// its private copy (churning the refcount and realloc-ing its block). The
// shared block is only ever read here, so TSan must stay silent; if any path
// wrote to a shared block, the reader reads would race with it.
TEST_CASE("Concurrent: byteArray_shared_read_while_cow")
{
    const size_t N = 8;
    const size_t READS = 20000;
    const size_t GROWS = 20000;

    ByteArray source(100, 'a');
    const size_t baseLen = source.size();

    std::atomic<bool> failed{false};
    std::vector<std::thread> threads;

    for (size_t t = 0; t < N; ++t) {
        threads.emplace_back([&source, baseLen, READS, &failed]() {
            ByteArray local(source);      // share source's block, read-only
            for (size_t i = 0; i < READS; ++i) {
                if (local.size() != baseLen ||
                    local[0] != 'a' ||
                    local[baseLen - 1] != 'a' ||
                    !local.startsWith("aaaa")) {
                    failed.store(true);
                    return;
                }
            }
        });
    }
    threads.emplace_back([&source, GROWS]() {
        ByteArray w(source);              // shares, then COW-diverges
        const char chunk[4] = {'w', 'r', 'i', 't'};
        for (size_t i = 0; i < GROWS; ++i) {
            w.append(chunk, 4);
        }
    });
    for (auto & th : threads) th.join();

    REQUIRE(!failed.load());
    REQUIRE_EQ(source.size(), baseLen);   // shared block never mutated
}

}

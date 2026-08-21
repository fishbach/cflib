/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "alloc_counter.h"

#include <atomic>
#include <cstdlib>
#include <new>

namespace cfbench {

namespace detail {

std::atomic<bool> counting{true};

std::atomic<long long> total{0};
std::atomic<long long> frees{0};
std::atomic<long long> live{0};
std::atomic<long long> peakLive{0};
std::atomic<long long> hist[16];

long long sizeClassImpl(size_t n)
{
    // bucket 0: <16; buckets 1..6: factor of 2 up to 1K;
    // buckets 7..15: factor of 4
    if (n < 16) return 0;
    unsigned lg = 63u - (unsigned)__builtin_clzll((unsigned long long)n);
    long long i = (lg < 10) ? (long long)lg - 3 : 7 + ((long long)lg - 10) / 2;
    if (i > 15) i = 15;
    return i;
}

void noteAlloc(size_t n)
{
    total.fetch_add(1, std::memory_order_relaxed);
    hist[sizeClassImpl(n)].fetch_add(1, std::memory_order_relaxed);
    long long l = live.fetch_add(1, std::memory_order_relaxed) + 1;
    for (;;) {
        long long p = peakLive.load(std::memory_order_relaxed);
        if (l <= p) break;
        if (peakLive.compare_exchange_weak(p, l, std::memory_order_relaxed)) break;
    }
}

void noteFree()
{
    frees.fetch_add(1, std::memory_order_relaxed);
    live.fetch_sub(1, std::memory_order_relaxed);
}

} // namespace detail

long long sizeClass(size_t n) { return detail::sizeClassImpl(n); }

const char * sizeClassName(long long i)
{
    static const char * names[16] = {
        "<16",      "[16,32)",  "[32,64)",   "[64,128)",  "[128,256)", "[256,512)",
        "[512,1K)", "[1K,4K)",  "[4K,16K)",  "[16K,64K)", "[64K,256K)","[256K,1M)",
        "[1M,4M)",  "[4M,16M)", "[16M,64M)", ">=64M"
    };
    if (i < 0) i = 0;
    if (i > 15) i = 15;
    return names[i];
}

Snapshot snapshot()
{
    Snapshot s;
    for (int i = 0; i < 16; ++i) s.hist[i] = detail::hist[i].load(std::memory_order_relaxed);
    s.total    = detail::total.load(std::memory_order_relaxed);
    s.frees    = detail::frees.load(std::memory_order_relaxed);
    s.live     = detail::live.load(std::memory_order_relaxed);
    s.peakLive = detail::peakLive.load(std::memory_order_relaxed);
    // restart peak tracking from the current live level, so that
    // s.peakLive - s.live of a later snapshot is the extra concurrency
    // reached in between
    detail::peakLive.store(s.live, std::memory_order_relaxed);
    return s;
}

void setCounting(bool on)
{
    detail::counting.store(on, std::memory_order_release);
}

} // namespace cfbench

// --- counting the malloc family ---------------------------------------------
// ByteArray allocates via std::malloc / std::realloc / std::free directly,
// so the allocator family is interposed as well. The real libc
// implementations are resolved via dlsym(RTLD_NEXT); the operator new/delete
// overrides below call them directly, so nothing is counted twice.
//
// Under ASan this interposition must not happen (ASan provides its own
// malloc), so it is compiled out there.

#ifndef __SANITIZE_ADDRESS__

#include <dlfcn.h>

namespace {

void * (* real_malloc)(size_t) = nullptr;
void (* real_free)(void *) = nullptr;
void * (* real_realloc)(void *, size_t) = nullptr;
void * (* real_calloc)(size_t, size_t) = nullptr;

void initRealAllocators()
{
    if (!real_malloc) {
        real_malloc  = (void *(*)(size_t))dlsym(RTLD_NEXT, "malloc");
        real_free    = (void (*)(void *))dlsym(RTLD_NEXT, "free");
        real_realloc = (void *(*)(void *, size_t))dlsym(RTLD_NEXT, "realloc");
        real_calloc  = (void *(*)(size_t, size_t))dlsym(RTLD_NEXT, "calloc");
    }
}

} // namespace

static inline void * rawMalloc(size_t n)
{
    initRealAllocators();
    return real_malloc(n ? n : 1);
}

static inline void rawFree(void * p)
{
    if (p) {
        initRealAllocators();
        real_free(p);
    }
}

extern "C" {

void * malloc(size_t n)
{
    void * p = rawMalloc(n);
    if (!p) std::terminate();
    if (cfbench::detail::counting.load(std::memory_order_acquire))
        cfbench::detail::noteAlloc(n);
    return p;
}

void * calloc(size_t n, size_t sz)
{
    initRealAllocators();
    void * p = real_calloc(n, sz);
    if (!p) std::terminate();
    if (cfbench::detail::counting.load(std::memory_order_acquire))
        cfbench::detail::noteAlloc(n * sz);
    return p;
}

void * realloc(void * p, size_t n)
{
    void * r = p ? real_realloc(p, n ? n : 1) : rawMalloc(n);
    if (!r) std::terminate();
    if (cfbench::detail::counting.load(std::memory_order_acquire)) {
        cfbench::detail::noteAlloc(n);
        if (p) cfbench::detail::noteFree();    // old block gone, new one live
    }
    return r;
}

void free(void * p)
{
    if (!p) return;
    rawFree(p);
    if (cfbench::detail::counting.load(std::memory_order_acquire))
        cfbench::detail::noteFree();
}

} // extern "C"

#else // __SANITIZE_ADDRESS__

static inline void * rawMalloc(size_t n) { return std::malloc(n ? n : 1); }
static inline void  rawFree(void * p)    { if (p) std::free(p); }

#endif // __SANITIZE_ADDRESS__

// --- counting global operators ---------------------------------------------
// This target is built with -fno-exceptions (cf_app default), so failure
// calls std::terminate() instead of throwing std::bad_alloc.

static inline void * countAlloc(size_t n)
{
    void * p = rawMalloc(n);
    if (!p) std::terminate();
    if (cfbench::detail::counting.load(std::memory_order_acquire))
        cfbench::detail::noteAlloc(n);
    return p;
}

void * operator new(size_t n)
{
    return countAlloc(n);
}

void * operator new[](size_t n)
{
    return countAlloc(n);
}

void * operator new(size_t n, const std::nothrow_t &) noexcept
{
    void * p = rawMalloc(n);
    if (p && cfbench::detail::counting.load(std::memory_order_acquire))
        cfbench::detail::noteAlloc(n);
    return p;
}

void * operator new(size_t n, std::align_val_t al)
{
    size_t bytes = n ? n : 1;
    void * p = nullptr;
    size_t align = (size_t)al;
    if (align > alignof(std::max_align_t)) {
        bytes = (bytes + align - 1) & ~(align - 1);
        if (posix_memalign(&p, align, bytes)) std::terminate();
    } else {
        p = rawMalloc(bytes);
        if (!p) std::terminate();
    }
    if (cfbench::detail::counting.load(std::memory_order_acquire))
        cfbench::detail::noteAlloc(n);
    return p;
}

void operator delete(void * p) noexcept {
    if (p) {
        rawFree(p);
        if (cfbench::detail::counting.load(std::memory_order_acquire))
            cfbench::detail::noteFree();
    }
}

void operator delete(void * p, size_t) noexcept {
    if (p) {
        rawFree(p);
        if (cfbench::detail::counting.load(std::memory_order_acquire))
            cfbench::detail::noteFree();
    }
}

void operator delete[](void * p) noexcept {
    if (p) {
        rawFree(p);
        if (cfbench::detail::counting.load(std::memory_order_acquire))
            cfbench::detail::noteFree();
    }
}

void operator delete[](void * p, size_t) noexcept {
    if (p) {
        rawFree(p);
        if (cfbench::detail::counting.load(std::memory_order_acquire))
            cfbench::detail::noteFree();
    }
}

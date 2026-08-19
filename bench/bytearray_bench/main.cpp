/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

// Micro-benchmarks for cflib::base::ByteArray and its String subclass.
//
// The counting allocator (alloc_counter.cpp) intercepts every global
// operator new/delete and malloc/calloc/realloc/free call of this
// process, so "allocs/op" below are real heap allocations, not
// estimates. Each scenario runs two passes:
// one with counting enabled (allocations) and one with counting
// disabled (timing, free of counter overhead).
//
//   bin/bytearray_bench

#include <cflib/base/bytearray.h>

#include "alloc_counter.h"

#include <chrono>
#include <cstdio>
#include <string>

using namespace cflib;

namespace {

void doNotOptimizeAway(size_t v)
{
    #if defined(__GNUC__) || defined(__clang__)
    asm volatile("" : : "r,m"(v) : );
    #else
    volatile size_t sink = v;
    (void)sink;
    #endif
}

struct Stats
{
    double nsPerOp;
    double allocsPerOp;
    long long peakLive;
};

template <typename F>
Stats run(const char * name, long long iters, F && f, int warmupIters = 100)
{
    // pass 1: allocation profile
    cfbench::setCounting(true);
    for (int i = 0; i < warmupIters; ++i) f(i);
    auto s0 = cfbench::snapshot();
    for (long long i = 0; i < iters; ++i) f(i);
    auto s1 = cfbench::snapshot();
    double allocsPerOp = (double)(s1.total - s0.total) / (double)iters;
    long long peakLive = s1.peakLive - s0.live;

    // pass 2: timing without counter overhead
    cfbench::setCounting(false);
    for (int i = 0; i < warmupIters; ++i) f(i);
    auto t0 = std::chrono::steady_clock::now();
    for (long long i = 0; i < iters; ++i) f(i);
    auto t1 = std::chrono::steady_clock::now();
    cfbench::setCounting(true);
    double ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count() / (double)iters;

    printf("%-34s %10lld %12.2f %10.4f %8lld\n", name, iters, ns, allocsPerOp, peakLive);
    int first = -1;
    for (int i = 0; i < 16; ++i) {
        long long d = s1.hist[i] - s0.hist[i];
        if (!d) continue;
        if (first < 0) { printf("    %-34s", ""); first = i; }
        printf("%7.3f x %-10s", (double)d / (double)iters, cfbench::sizeClassName(i));
    }
    if (first >= 0) printf("\n");

    Stats st;
    st.nsPerOp = ns;
    st.allocsPerOp = allocsPerOp;
    st.peakLive = peakLive;
    return st;
}

std::string makeContent(size_t n, char base)
{
    std::string s(n, ' ');
    for (size_t i = 0; i < n; ++i) s[i] = (char)(base + (i % 26));
    return s;
}

} // namespace

int main()
{
    const std::string shortS = makeContent(100, 'a');        // 100 B
    const std::string midS   = makeContent(10 * 1024, 'a');  // 10 KB
    const std::string bigS   = makeContent(100 * 1024, 'a'); // 100 KB

    ByteArray mid(midS.c_str());
    ByteArray big(bigS.c_str());

    printf("bytearray_bench (heap allocations counted via new/malloc)\n");
    printf("%-34s %10s %12s %10s %8s\n", "scenario", "iters", "ns/op", "allocs/op", "peakLive");

    run("null: default ctor/dtor", 1000000, [](int) {
        ByteArray ba;
        doNotOptimizeAway((size_t)ba.isNull());
    });

    run("null: append 1 byte", 1000000, [](int) {
        ByteArray ba;
        ba.append('x');
        doNotOptimizeAway(ba.size());
    });

    run("short 100B: create+5 copies+mutate", 200000, [&](int) {
        ByteArray a(shortS.c_str());
        ByteArray b(a), c(a), d(a), e(a), f(a);
        b.append('!');
        doNotOptimizeAway(a.size() + b.size());
    });

    run("long 10KB: create+5 copies+mutate", 20000, [&](int) {
        ByteArray a(midS.c_str());
        ByteArray b(a), c(a), d(a), e(a), f(a);
        b.append('!');
        doNotOptimizeAway(a.size() + b.size());
    });

    run("grow: append 1B until 1 MiB (single pass)", 1, [&](int) {
        ByteArray a;
        for (size_t i = 0; i < (size_t)(1 << 20); ++i) a.append('a');
        doNotOptimizeAway(a.size());
    }, /*warmup*/ 2);

    run("indexOf: full scan of 100KB (absent)", 50000, [&](int) {
        doNotOptimizeAway((size_t)big.indexOf((char)0x01) + 1);
    });

    run("toStdString(): copy 10KB", 100000, [&](int) {
        std::string s = mid.toStdString();
        doNotOptimizeAway(s.size());
    });

    run("split(','): 8 fields", 200000, [](int) {
        auto parts = ByteArray("a,b,c,d,e,f,g,h").split(',');
        doNotOptimizeAway(parts.size());
    });

    run("String 100B: ctor/charCount/dtor", 200000, [&](int) {
        String s(shortS.c_str());
        doNotOptimizeAway(s.charCount());
    });

    run("operator== char* (mismatch)", 1000000, [&](int) {
        doNotOptimizeAway((size_t)(mid == "definitely a different string"));
    });

    return 0;
}

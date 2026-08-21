/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

// Counting allocator for benchmarks. alloc_counter.cpp interposes the
// global operator new/delete and the malloc family (malloc/calloc/
// realloc/free), so every heap allocation of this process is counted
// regardless of which API it goes through. Counting is switchable at
// runtime: scenarios measure allocations (counting on) and timing
// (counting off) in separate passes, so counter overhead does not
// inflate ns/op. Under ASan the malloc interposition is disabled.

#pragma once

#include <cstddef>

namespace cfbench {

struct Snapshot
{
    long long total;      // cumulative allocations
    long long frees;      // cumulative frees
    long long live;       // currently live
    long long peakLive;   // highest `live` ever reached
    long long hist[16];   // allocations per size class
};

Snapshot snapshot();

// size class index for a given allocation size (see sizeClassName)
long long sizeClass(size_t n);
// human-readable size class name, e.g. "[64,128)"
const char * sizeClassName(long long i);

void setCounting(bool on);

} // namespace cfbench

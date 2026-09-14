/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

// The grow loop of the "grow: append 1B until 1 MiB" scenario. Kept in
// its own translation unit so it is not inlined into main(): inlined
// there, its codegen depends on main()'s size (once main() gets large,
// GCC stops inlining ByteArray::detach into the append call), and the
// single shot's timing jumps ~2x when unrelated scenarios are added.
// From its own TU it compiles in a small, stable context -- the
// codegen a normal (non-benchmark) program gets.

#include <cflib/base/bytearray.h>

using namespace cflib::base;

size_t growTo1MiB()
{
    ByteArray a;
    for (size_t i = 0; i < (size_t)(1 << 20); ++i) a.append('a');
    return a.size();
}

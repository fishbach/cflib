/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "timer.h"

#include <cflib/util/libev.h>
#include <cflib/util/threadverify.h>

namespace cflib::util {

void Timer::singleShot(double afterSecs, const Functor * func)
{
    ev_loop * loop = libEVLoopOfThread();
    if (loop) ev_once(loop, -1, 0, afterSecs, &Timer::timeout, (void *)func);
    else {
        // No libev loop on this thread - just execute immediately
        (*func)();
        delete func;
    }
}

void Timer::timeout(int, void * arg)
{
    const Functor * func = (const Functor *)arg;
    (*func)();
    delete func;
}

} // namespace

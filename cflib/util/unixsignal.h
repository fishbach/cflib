/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/util/sig.h>

struct ev_loop;
struct ev_io;

namespace cflib::util {

// catches signals 1, 2 and 15
class UnixSignal
{
public:
    UnixSignal(bool exitMainLoop = false);
    ~UnixSignal();

cfsignals:
    sig<void (int)> catchedSignal;

private:
    static void ioCallback(ev_loop * loop, ev_io * w, int revents);
    void gotSignal(int sig);

private:
    const bool exitMainLoop_;
    ev_io * watcher_;
};

} // namespace

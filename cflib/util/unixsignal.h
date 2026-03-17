/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/util/sig.h>

struct ev_loop;
struct ev_io;

namespace cflib { namespace util {

// catches signals 1, 2 and 15
class UnixSignal
{
public:
    UnixSignal();
    ~UnixSignal();

    cfsignals:
        sig<void (int)> catchedSignal;

private:
    static void ioCallback(ev_loop * loop, ev_io * w, int revents);

    ev_io * watcher_;
};

}}    // namespace

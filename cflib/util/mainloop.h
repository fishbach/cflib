/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/util/impl/threadverifyimpl.h>

namespace cflib::util {

class MainLoop
{
public:
    MainLoop();
    ~MainLoop();

    int exec();

    static void quit(int exitCode = 0);

private:
    static inline MainLoop * instance_ = nullptr;
    ThreadFifo<const Functor *> externalCalls_;
    impl::LibEVThreadLoop loop_;
    AtomicInt exitCode_ = 0;
};

} // namespace

/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "mainloop.h"

#include <cflib/util/log.h>

USE_LOG(LogCat::Etc)

namespace cflib::util {

MainLoop::MainLoop() :
    externalCalls_(1024),
    loop_("main", externalCalls_, true)
{
    if (!instance_) instance_ = this;
    loop_.assignToCurrentThread();
}

MainLoop::~MainLoop()
{
    if (instance_ == this) instance_ = nullptr;
}

int MainLoop::exec()
{
    loop_.run();
    return exitCode_.loadAcquire();
}

void MainLoop::quit(int exitCode)
{
    if (!instance_) {
        logWarn("no MainLoop instance to call quit()");
        return;
    }
    instance_->exitCode_.storeRelease(exitCode);
    instance_->loop_.stopLoop();
}

} // namespace

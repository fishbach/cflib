/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "threadverify.h"

#include <cflib/util/libev.h>
#include <cflib/util/log.h>

USE_LOG(LogCat::Etc)

namespace cflib::util {

ThreadVerify::ThreadVerify(const String & threadName, LoopType loopType, uint threadCount) :
    ownerOfVerifyThread_(true)
{
    verifyThread_ = new impl::ThreadHolder(threadName, loopType == Worker, threadCount);
}

ThreadVerify::ThreadVerify(ThreadVerify * other) :
    verifyThread_(other->verifyThread_),
    ownerOfVerifyThread_(false)
{
}

ThreadVerify::ThreadVerify() :
    verifyThread_(0),
    ownerOfVerifyThread_(true)
{
}

ThreadVerify::~ThreadVerify()
{
    if (ownerOfVerifyThread_) {
        delete verifyThread_;
    }
}

void ThreadVerify::stopVerifyThread()
{
    if (ownerOfVerifyThread_ && verifyThread_->isActive()) {
        shutdownThread();
        verifyThread_->stopLoop();
    }
}

void ThreadVerify::execCall(const Functor * func) const
{
    if (!verifyThread_->isActive()) {
        logCritical("execCall for already terminated thread %1", verifyThread_->name());
        delete func;
        return;
    }
    while (!verifyThread_->doCall(func)) Thread::sleep(1);
}

void ThreadVerify::execLater(const Functor * func) const
{
    // We cannot use verifyThread_ here, as we could have a thread pool, where verifyThread_->execLater() could
    // start immediately.
    const impl::LibEVThreadLoop * thread = dynamic_cast<const impl::LibEVThreadLoop *>(Thread::current());
    if (thread) thread->execLater(func);
}

void ThreadVerify::execLater(const std::function<void ()> & func) const
{
    execLater(new StdFunctor(func));
}

void ThreadVerify::shutdownThread()
{
    if (!verifyThreadCall(&ThreadVerify::shutdownThread)) return;

    logFunctionTrace
    deleteThreadData();
}

} // namespace

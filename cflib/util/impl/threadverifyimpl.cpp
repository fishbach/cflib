/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "threadverifyimpl.h"

#include <cflib/util/libev.h>
#include <cflib/util/log.h>

USE_LOG(LogCat::Etc)

namespace cflib::util::impl {

LibEVThreadLoop::LibEVThreadLoop(const String & threadName,
    ThreadFifo<const Functor *> & externalCalls, bool isWorkerOnly)
:
    Thread(threadName),
    externalCalls_(externalCalls),
    loop_(ev_loop_new((uint)EVFLAG_NOSIGMASK | (isWorkerOnly ? EVBACKEND_SELECT : EVBACKEND_ALL))),
    wakeupWatcher_(new ev_async)
{
    ev_async_init(wakeupWatcher_, &LibEVThreadLoop::asyncCallback);
    wakeupWatcher_->data = this;
    ev_async_start(loop_, wakeupWatcher_);
}

LibEVThreadLoop::~LibEVThreadLoop()
{
    logTrace("~LibEVThreadLoop()");
    ev_async_stop(loop_, wakeupWatcher_);
    wakeupWatcher_->data = 0;
    delete wakeupWatcher_;
    ev_loop_destroy(loop_);
}

void LibEVThreadLoop::wakeUp()
{
    ev_async_send(loop_, wakeupWatcher_);
}

void LibEVThreadLoop::stopLoop()
{
    stopLoop_.storeRelease(true);
    wakeUp();
}

void LibEVThreadLoop::execLater(const Functor * func) const
{
    ev_once(loop_, -1, 0, 0.0, &LibEVThreadLoop::execLaterCall, (void *)func);
}

void LibEVThreadLoop::run()
{
    logDebug("loop %1 started with libev backend %2", name(), (uint32)ev_backend(loop_));
    ev_run(loop_, 0);
    logDebug("loop %1 stopped", name());
}

void LibEVThreadLoop::wokeUp()
{
    while (const Functor * func = externalCalls_.take()) {
        (*func)();
        delete func;
    }

    if (stopLoop_.loadAcquire()) ev_break(loop_, EVBREAK_ALL);
}


void LibEVThreadLoop::asyncCallback(ev_loop *, ev_async * w, int)
{
    ((LibEVThreadLoop *)w->data)->wokeUp();
}

void LibEVThreadLoop::execLaterCall(int, void * arg)
{
    const Functor * func = (const Functor *)arg;
    (*func)();
    delete func;
}

ThreadHolder::ThreadHolder(const String & threadName, bool isWorkerOnly, uint threadCount) :
    threadName_(threadName),
    externalCalls_(1024)
{
    if (!isWorkerOnly && threadCount > 1) {
        logCritical("thread count must be less or equal 1 for network thread %1", threadName);
        threadCount = 1;
    }
    for (uint i = 1 ; i <= threadCount ; ++i) {
        const String workerName = threadCount == 1 ?
            threadName :
            threadName + " " + String::number(i) + "/" + String::number(threadCount);
        LibEVThreadLoop * worker = new LibEVThreadLoop(workerName, externalCalls_, isWorkerOnly);
        worker->start();
        workers_ << worker;
    }
}

ThreadHolder::~ThreadHolder()
{
    if (!finished_.loadAcquire()) {
        logCritical("thread %1 has not been stopped before destruction", threadName_);
    }
    for (LibEVThreadLoop * w : workers_) delete w;
}

bool ThreadHolder::doCall(const Functor * func)
{
    if (!externalCalls_.put(func)) {
        const Thread * thread = Thread::current();
        logWarn("queue of thread %1 full (called by %2 (%3))",
            name(),
            thread ? thread->name() : "?",
            thread ? thread->id() : 0);
        return false;
    }
    for (LibEVThreadLoop * w : workers_) w->wakeUp();
    return true;

}

void ThreadHolder::stopLoop()
{
    isActive_.storeRelease(false);
    for (LibEVThreadLoop * w : workers_) w->stopLoop();
    for (LibEVThreadLoop * w : workers_) w->join();
    finished_.storeRelease(true);
}

bool ThreadHolder::isOwnThread() const
{
    if (workers_.empty()) return true;
    const Thread * own = Thread::current();
    for (const LibEVThreadLoop * w : workers_) if (own == w) return true;
    return false;
}

} // namespace

/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "threadverifyimpl.h"

#include <cflib/util/libev.h>
#include <cflib/util/log.h>
#include <cflib/util/threadstats.h>

#include <cstdio>

USE_LOG(LogCat::Etc)

// Definition of the thread-local pointer declared in cfthread.h
thread_local cflib::util::impl::ThreadHolder * cf_current_thread = nullptr;

namespace cflib::util::impl {

ThreadHolder::ThreadHolder(const String & threadName, int threadId, ThreadStats * stats, bool disable) :
    threadName(threadName),
    threadId_(threadId), stats_(stats),
    disabled_(disable), isActive_(true), isRunning_(false)
{
}

ThreadHolder::~ThreadHolder()
{
    logTrace("~ThreadHolder()");
}

void ThreadHolder::startThread()
{
    isRunning_ = true;
    thread_ = std::thread([this]() {
        cf_current_thread = this;
        run();
        isRunning_ = false;
    });
}

void ThreadHolder::join()
{
    if (thread_.joinable()) thread_.join();
}

ThreadHolderLibEV::ThreadHolderLibEV(const String & threadName, int threadId, ThreadStats * stats, bool isWorkerOnly, bool disable) :
    ThreadHolder(threadName, threadId, stats, disable),
    loop_(ev_loop_new((cfuint)EVFLAG_NOSIGMASK | (isWorkerOnly ? EVBACKEND_SELECT : EVBACKEND_ALL))),
    wakeupWatcher_(new ev_async)
{
    ev_async_init(wakeupWatcher_, &ThreadHolderLibEV::asyncCallback);
    wakeupWatcher_->data = this;
    ev_async_start(loop_, wakeupWatcher_);
}

ThreadHolderLibEV::~ThreadHolderLibEV()
{
    ev_async_stop(loop_, wakeupWatcher_);
    wakeupWatcher_->data = 0;
    delete wakeupWatcher_;
    ev_loop_destroy(loop_);
}

void ThreadHolderLibEV::stopLoop()
{
    ev_break(loop_, EVBREAK_ALL);
}

void ThreadHolderLibEV::execLater(const Functor * func) const
{
    ev_once(loop_, -1, 0, 0.0, &ThreadHolderLibEV::execLaterCall, (void *)func);
}

void ThreadHolderLibEV::execLaterCall(int, void * arg)
{
    const Functor * func = (const Functor *)arg;
    (*func)();
    delete func;
}

void ThreadHolderLibEV::wakeUp()
{
    ev_async_send(loop_, wakeupWatcher_);
}

void ThreadHolderLibEV::run()
{
    logDebug("thread %1 started with libev backend %2", threadName, (cfuint32)ev_backend(loop_));
    ev_run(loop_, 0);
    isActive_ = false;
    logDebug("thread %1 stopped", threadName);
}

void ThreadHolderLibEV::asyncCallback(ev_loop *, ev_async * w, int)
{
    ((ThreadHolderLibEV *)w->data)->wokeUp();
}

ThreadHolderWorkerPool::ThreadHolderWorkerPool(const String & threadName,
    int threadId, ThreadStats * stats, bool isWorkerOnly, cfuint threadCount)
:
    ThreadHolderLibEV(threadCount > 1 ?
        String(threadName.str() + " 1/" + std::to_string(threadCount)) : threadName,
        threadId, stats, isWorkerOnly, threadCount == 0),
    externalCalls_(1024),
    stopLoop_(false)
{
    if (!disabled_) startThread();
    for (cfuint i = 2 ; i <= threadCount ; ++i) {
        String workerName(threadName.str() + " " + std::to_string(i) + "/" + std::to_string(threadCount));
        Worker * thread = new Worker(workerName, threadId, stats, i - 1, externalCalls_);
        workers_.push_back(thread);
    }
}

ThreadHolderWorkerPool::~ThreadHolderWorkerPool()
{
    for (Worker * w : workers_) delete w;
}

bool ThreadHolderWorkerPool::doCall(const Functor * func)
{
    if (!externalCalls_.put(func)) {
        if (stats_) stats_->externOverflow(threadId_);
        const impl::ThreadHolder * thread = cf_current_thread;
        logWarn("queue of thread %1 full (called by %2)", threadName, thread ? thread->threadName : String("?"));
        return false;
    }
    wakeUp();
    for (Worker * w : workers_) w->wakeUp();
    return true;
}

void ThreadHolderWorkerPool::stopLoop()
{
    if (!disabled_) {
        stopLoop_ = true;
        wakeUp();
        for (Worker * w : workers_) w->stopLoop();
    } else {
        isActive_ = false;
    }
}

bool ThreadHolderWorkerPool::isOwnThread() const
{
    if (cf_current_thread == this || disabled_) return true;
    for (const Worker * w : workers_) if (cf_current_thread == w) return true;
    return false;
}

cfuint ThreadHolderWorkerPool::threadCount() const
{
    return 1 + (cfuint)workers_.size();
}

void ThreadHolderWorkerPool::wokeUp()
{
    if (stopLoop_) {
        ThreadHolderLibEV::stopLoop();
        return;
    }

    CFElapsedTimer elapsed;
    if (stats_) elapsed.start();
    while (const Functor * func = externalCalls_.take()) {
        (*func)();
        delete func;
    }
    if (stats_) stats_->externNewCallTime(threadId_, elapsed.nsecsElapsed());
}

void ThreadHolderWorkerPool::run()
{
    ThreadHolderLibEV::run();
    for (Worker * w : workers_) w->join();
}

ThreadHolderWorkerPool::Worker::Worker(const String & threadName,
    int threadId, ThreadStats * stats, cfuint threadNo, ThreadFifo<const Functor *> & externalCalls)
:
    ThreadHolderLibEV(threadName, threadId, stats, true, false),
    threadNo_(threadNo),
    externalCalls_(externalCalls),
    stopLoop_(false)
{
    startThread();
}

void ThreadHolderWorkerPool::Worker::stopLoop()
{
    stopLoop_ = true;
    wakeUp();
}

void ThreadHolderWorkerPool::Worker::wokeUp()
{
    if (stopLoop_) {
        ThreadHolderLibEV::stopLoop();
        return;
    }

    while (const Functor * func = externalCalls_.take()) {
        (*func)();
        delete func;
    }
}

} // namespace

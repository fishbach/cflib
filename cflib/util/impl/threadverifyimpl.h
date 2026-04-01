/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>
#include <cflib/util/functor.h>
#include <cflib/util/thread.h>
#include <cflib/util/threadfifo.h>

struct ev_async;
struct ev_loop;

namespace cflib::util {

class MainLoop;

namespace impl {

class LibEVThreadLoop : public Thread
{
public:
    LibEVThreadLoop(const String & threadName, ThreadFifo<const Functor *> & externalCalls, bool isWorkerOnly);
    ~LibEVThreadLoop();

    void wakeUp();
    void stopLoop();
    void execLater(const Functor * func) const;

    ev_loop * loop() const { return loop_; }

protected:
    void run() override;
    void wokeUp();

private:
    static void asyncCallback(ev_loop * loop, ev_async * w, int revents);
    static void execLaterCall(int revents, void * arg);

private:
    ThreadFifo<const Functor *> & externalCalls_;
    ev_loop * loop_;
    ev_async * wakeupWatcher_;
    AtomicBool stopLoop_ = false;
    AtomicBool loopFinished_ = false;

    friend class cflib::util::MainLoop;
};

class ThreadHolder
{
    CF_DISABLE_COPY(ThreadHolder)
public:
    ThreadHolder(const String & threadName, bool isWorkerOnly, uint threadCount);
    ~ThreadHolder();

    String name() const { return threadName_; }
    bool isActive() const { return isActive_.loadAcquire(); }
    bool doCall(const Functor * func);
    void stopLoop();
    bool isOwnThread() const;

private:
    const String threadName_;
    ThreadFifo<const Functor *> externalCalls_;
    List<LibEVThreadLoop *> workers_;
    AtomicBool isActive_ = true;
    AtomicBool finished_ = false;
};

}} // namespace

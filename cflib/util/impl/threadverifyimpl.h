/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>
#include <cflib/util/functor.h>
#include <cflib/util/threadfifo.h>

#include <thread>
#include <vector>

struct ev_async;
struct ev_loop;

namespace cflib { namespace util {

class ThreadStats;

namespace impl {

class ThreadHolder
{
public:
    ThreadHolder(const String & threadName, int threadId, ThreadStats * stats, bool disable);
    virtual ~ThreadHolder();

    const String threadName;
    bool isActive() const { return isActive_; }
    bool isRunning() const { return isRunning_; }
    virtual bool doCall(const Functor * func) = 0;
    virtual void stopLoop() = 0;
    virtual bool isOwnThread() const { return cf_current_thread == this || disabled_; }
    virtual cfuint threadCount() const { return 1; }
    virtual cfuint threadNo() const    { return 0; }
    virtual void execLater(const Functor * func) const = 0;

    void startThread();
    void join();

protected:
    virtual void run() = 0;

    const int threadId_;
    ThreadStats * const stats_;
    const bool disabled_;
    bool isActive_;
    bool isRunning_;

private:
    std::thread thread_;
};

class ThreadHolderLibEV : public ThreadHolder
{
public:
    ~ThreadHolderLibEV();

    void stopLoop() override;
    void execLater(const Functor * func) const override;
    ev_loop * loop() const { return loop_; }
    void wakeUp();

protected:
    ThreadHolderLibEV(const String & threadName, int threadId, ThreadStats * stats, bool isWorkerOnly, bool disable);
    void run() override;
    virtual void wokeUp() = 0;

private:
    static void asyncCallback(ev_loop * loop, ev_async * w, int revents);
    static void execLaterCall(int revents, void * arg);

private:
    ev_loop * loop_;
    ev_async * wakeupWatcher_;
};

class ThreadHolderWorkerPool : public ThreadHolderLibEV
{
public:
    ThreadHolderWorkerPool(const String & threadName, int threadId, ThreadStats * stats, bool isWorkerOnly, cfuint threadCount);
    ~ThreadHolderWorkerPool();

    bool doCall(const Functor * func) override;
    void stopLoop() override;
    bool isOwnThread() const override;
    cfuint threadCount() const override;

protected:
    void run() override;
    void wokeUp() override;

private:
    class Worker : public ThreadHolderLibEV
    {
    public:
        Worker(const String & threadName,
            int threadId, ThreadStats * stats, cfuint threadNo, ThreadFifo<const Functor *> & externalCalls);

        bool doCall(const Functor *) override { return false; }
        void stopLoop() override;
        cfuint threadNo() const override { return threadNo_; }

    protected:
        void wokeUp() override;

    private:
        const cfuint threadNo_;
        ThreadFifo<const Functor *> & externalCalls_;
        bool stopLoop_;
    };

    ThreadFifo<const Functor *> externalCalls_;
    std::vector<Worker *> workers_;
    bool stopLoop_;
};

}}}    // namespace

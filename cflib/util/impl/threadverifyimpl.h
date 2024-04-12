/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/util/functor.h>
#include <cflib/util/threadfifo.h>

struct ev_async;
struct ev_loop;

namespace cflib { namespace util {

class ThreadStats;

namespace impl {

class ThreadHolderEvent : public QEvent
{
public:
    ThreadHolderEvent(const Functor * func) :
        QEvent(QEvent::User),
        func(func)
    {}

    const Functor * func;
};

class ThreadObject : public QObject
{
public:
    ThreadObject(int threadId, ThreadStats * stats);
    bool event(QEvent * event) override;

private:
    const int threadId_;
    ThreadStats * const stats_;
};

class ThreadHolder : public QThread
{
public:
    ThreadHolder(const QString & threadName, int threadId, ThreadStats * stats, bool disable);
    ~ThreadHolder();

    const QString threadName;
    bool isActive() const { return isActive_; }
    virtual bool doCall(const Functor * func) = 0;
    virtual void stopLoop() = 0;
    virtual bool isOwnThread() const { return QThread::currentThread() == this || disabled_; }
    virtual uint threadCount() const { return 1; }
    virtual uint threadNo() const    { return 0; }
    virtual void execLater(const Functor * func) const = 0;

protected:
    const int threadId_;
    ThreadStats * const stats_;
    const bool disabled_;
    bool isActive_;
};

class ThreadHolderQt : public ThreadHolder
{
public:
    ThreadHolderQt(const QString & threadName, int threadId, ThreadStats * stats, bool disable);

    ThreadObject * threadObject() const { return threadObject_; }
    bool doCall(const Functor * func) override;
    void stopLoop() override;
    void execLater(const Functor * func) const override;

protected:
    void run() override;

private:
    ThreadObject * threadObject_;
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
    ThreadHolderLibEV(const QString & threadName, int threadId, ThreadStats * stats, bool isWorkerOnly, bool disable);
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
    ThreadHolderWorkerPool(const QString & threadName, int threadId, ThreadStats * stats, bool isWorkerOnly, uint threadCount);
    ~ThreadHolderWorkerPool();

    bool doCall(const Functor * func) override;
    void stopLoop() override;
    bool isOwnThread() const override;
    uint threadCount() const override;

protected:
    void run() override;
    void wokeUp() override;

private:
    class Worker : public ThreadHolderLibEV
    {
    public:
        Worker(const QString & threadName,
            int threadId, ThreadStats * stats, uint threadNo, ThreadFifo<const Functor *> & externalCalls);

        bool doCall(const Functor *) override { return false; }
        void stopLoop() override;
        uint threadNo() const override { return threadNo_; }

    protected:
        void wokeUp() override;

    private:
        const uint threadNo_;
        ThreadFifo<const Functor *> & externalCalls_;
        bool stopLoop_;
    };

    ThreadFifo<const Functor *> externalCalls_;
    QList<Worker *> workers_;
    bool stopLoop_;
};

}}}    // namespace

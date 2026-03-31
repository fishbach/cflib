/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>

#include <thread>

namespace cflib::base {

class Thread
{
    CF_DISABLE_COPY(Thread)
public:
    Thread(const String & threadName);
    virtual ~Thread();

    size_t id() const { return threadId_; }
    String name() const { return threadName_; }

    void start();
    void join();

    static Thread * current() { return threadPtr_; }
    static size_t currentId() { return threadPtr_ ? threadPtr_->threadId_ : 0; }

    static void  sleep(size_t      secs);
    static void msleep(size_t milliSecs);
    static void usleep(size_t microSecs);

protected:
    virtual void run() = 0;

private:
    void doRun();

private:
    static inline thread_local Thread * threadPtr_ = nullptr;
    static AtomicUInt lastThreadId_;
    const size_t threadId_;
    const String threadName_;
    std::jthread thread_;
    AtomicBool started_ = false;
    AtomicBool finished_ = false;
};

} // namespace

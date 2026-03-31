#include "thread.h"

#include <cflib/util/log.h>

USE_LOG(LogCat::Etc)

namespace cflib::base {

AtomicUInt Thread::lastThreadId_ = 0;

Thread::Thread(const String & threadName) :
    threadId_(++lastThreadId_),
    threadName_(threadName)
{
    logDebug("thread %1 (%2) created", threadName_, threadId_);
}

Thread::~Thread()
{
    if (started_.loadAcquire() && !finished_.loadAcquire()) {
        logInfo("thread %1 (%2) still running. Waiting for termination", threadName_, threadId_);
    }
    thread_.join();
    logDebug("thread %1 (%2) destroyed", threadName_, threadId_);
}

void Thread::start()
{
    if (started_.loadAcquire()) {
        logWarn("thread %1 (%2) started twice", threadName_, threadId_);
        return;
    }
    started_.storeRelease(true);
    thread_ = std::jthread([this](){ doRun(); });
}

void Thread::join()
{
    if (!started_.loadAcquire()) {
        logWarn("thread %1 (%2) not started, but joined", threadName_, threadId_);
        return;
    }
    thread_.join();
    thread_ = std::jthread();
    started_.storeRelease(false);
    finished_.storeRelease(false);
}

void Thread::doRun()
{
    threadPtr_ = this;
    logInfo("thread %1 (%2) started", threadName_, threadId_);
    run();
    logInfo("thread %1 (%2) stopped", threadName_, threadId_);
    finished_.storeRelease(true);
}

void Thread::sleep(size_t secs)
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void Thread::msleep(size_t milliSecs)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

void Thread::usleep(size_t microSecs)
{
    std::this_thread::sleep_for(std::chrono::microseconds(1));
}

} // namespace

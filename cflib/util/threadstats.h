/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/util/threadverify.h>
#include <cflib/util/evtimer.h>

namespace cflib { namespace util {

class ThreadStats : private ThreadVerify
{
    CF_DISABLE_COPY(ThreadStats)
public:
    struct ThreadInfo
    {
        String name;
        cfint64 current;
        cfint64 total;
        cfint64 peaks;
        cfint64 overflows;
        cfint64 avg;

        ThreadInfo() : current(0), total(0), peaks(0), overflows(0), avg(0) {}
    };
    typedef CFVector<ThreadInfo> ThreadInfos;

public:
    ThreadStats() : ThreadVerify(), timer_(this, &ThreadStats::timeout)
    {
        verifyThread_ = new impl::ThreadHolderWorkerPool("ThreadStats", -1, 0, true, 1);
        setStats(this);
        init();
    }

    ~ThreadStats()
    {
        stopVerifyThread();
    }

    inline void externNewCallTime(int threadId, cfint64 nsecs)
    {
        if (!verifyThreadCall(&ThreadStats::externNewCallTime, threadId, nsecs)) return;
        infos_[threadId].current += nsecs;
    }

    inline void externOverflow(int threadId)
    {
        if (!verifyThreadCall(&ThreadStats::externOverflow, threadId)) return;
        ++infos_[threadId].overflows;
    }

    int externNewId(const String & threadName)
    {
        SyncedThreadCall<int> stc(this);
        if (!stc.verify(&ThreadStats::externNewId, threadName)) return stc.retval();

        ThreadInfo info;
        info.name = threadName;
        infos_.push_back(info);
        return (int)infos_.size() - 1;
    }

    ThreadInfos current() const
    {
        SyncedThreadCall<ThreadInfos> stc(this);
        if (!stc.verify(&ThreadStats::current)) return stc.retval();
        return infos_;
    }

private:
    void init()
    {
        if (!verifyThreadCall(&ThreadStats::init)) return;

        timer_.start(1.0);
        elapsed_.start();
    }

    void timeout()
    {
        cfint64 dt = elapsed_.nsecsElapsed();
        for (ThreadInfo & info : infos_) {
            if (info.current > dt) {
                ++info.peaks;
                info.avg = 10000;
            } else {
                info.avg = info.current * 10000 / dt;
            }
            info.total += info.current;
            info.current = 0;
        }
        elapsed_.start();
    }

private:
    ThreadInfos infos_;
    EVTimer timer_;
    CFElapsedTimer elapsed_;
};

}}    // namespace

/* Copyright (C) 2013-2017 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * cflib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * cflib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cflib. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <cflib/util/threadverify.h>
#include <cflib/util/evtimer.h>

namespace cflib { namespace util {

class ThreadStats : private ThreadVerify
{
	Q_DISABLE_COPY(ThreadStats)
public:
	struct ThreadInfo
	{
		QString name;
		qint64 current;
		qint64 avg;
		qint64 peaks;
		qint64 total;

		ThreadInfo() : current(0), avg(0), peaks(0), total(0) {}
	};
	typedef QVector<ThreadInfo> ThreadInfos;

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

	inline void externNewCallTime(int threadId, qint64 nsecs)
	{
		if (!verifyThreadCall(&ThreadStats::externNewCallTime, threadId, nsecs)) return;
		infos_[threadId].current += nsecs;
	}

	int externNewId(const QString & threadName)
	{
		SyncedThreadCall<int> stc(this);
		if (!stc.verify(&ThreadStats::externNewId, threadName)) return stc.retval();

		ThreadInfo info;
		info.name = threadName;
		infos_ << info;
		return infos_.size() - 1;
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
		qint64 dt = elapsed_.nsecsElapsed();
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
	QElapsedTimer elapsed_;
};

}}	// namespace

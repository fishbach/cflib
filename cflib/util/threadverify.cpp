/* Copyright (C) 2013-2014 Christian Fischbach <cf@cflib.de>
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

#include "threadverify.h"

#include <cflib/libev/libev.h>
#include <cflib/util/log.h>

USE_LOG(LogCat::Etc)

namespace cflib { namespace util {

ThreadVerify::ThreadVerify(const QString & threadName, LoopType loopType, uint threadCount) :
	ownerOfVerifyThread_(true)
{
	if (loopType == Qt) {
		if (threadCount != 1) logCritical("thread count must be 1 for Qt thread %1", threadName);
		verifyThread_ = new impl::ThreadHolderQt(threadName);
	} else if (loopType == Net) {
		if (threadCount != 1) logCritical("thread count must be 1 for network thread %1", threadName);
		verifyThread_ = new impl::ThreadHolderWorkerPool(threadName, false);
	} else {
		if (threadCount == 0) logCritical("thread count cannot be 0 for worker thread %1", threadName);
		verifyThread_ = new impl::ThreadHolderWorkerPool(threadName, true, threadCount);
	}
}

ThreadVerify::ThreadVerify(ThreadVerify * other) :
	verifyThread_(other->verifyThread_),
	ownerOfVerifyThread_(false)
{
}

ThreadVerify::~ThreadVerify()
{
	if (ownerOfVerifyThread_) {
		if (verifyThread_->isActive()) {
			logCritical("thread %1 has not been stopped before destruction", verifyThread_->threadName);
		}
		verifyThread_->deleteLater();
	}
}

QObject * ThreadVerify::threadObject() const
{
	const impl::ThreadHolderQt * th = dynamic_cast<const impl::ThreadHolderQt *>(verifyThread_);
	if (!th) return 0;
	return th->threadObject();
}

void ThreadVerify::stopVerifyThread()
{
	if (ownerOfVerifyThread_ && verifyThread_->isRunning()) {
		shutdownThread();
		verifyThread_->wait();
	}
}

ev_loop * ThreadVerify::libEVLoop() const
{
	const impl::ThreadHolderLibEV * th = dynamic_cast<const impl::ThreadHolderLibEV *>(verifyThread_);
	if (!th) return 0;
	return th->loop();
}

namespace {

class Deleter : public Functor
{
public:
	Deleter(const ThreadVerify * obj) : obj_(obj) {}

	virtual void operator()() const
	{
		delete obj_;
	}

private:
	const ThreadVerify * obj_;
};

}

void ThreadVerify::deleteNext()
{
	execCall(new Deleter(this));
}

void ThreadVerify::execCall(const Functor * func)
{
	if (!verifyThread_->isActive()) {
		logWarn("execCall for already terminated thread %1", verifyThread_->threadName);
		return;
	}
	while (!verifyThread_->doCall(func)) {
		logWarn("Call queue of destination thread full! Waiting ...");
		QThread::msleep(1000);
	}
}

void ThreadVerify::shutdownThread()
{
	if (!verifyThreadCall(&ThreadVerify::shutdownThread)) return;

	logFunctionTrace
	deleteThreadData();
	verifyThread_->stopLoop();
}

}}	// namespace

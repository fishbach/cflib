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

#include <cflib/util/log.h>

USE_LOG(LogCat::Etc)

namespace cflib { namespace util {

ThreadVerify::ThreadVerify(const QString & threadName) :
	verifyThread_(new impl::ThreadHolder(threadName)),
	ownerOfVerifyThread_(true)
{
	verifyThread_->start();
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
			logWarn("thread %1 has not been stopped before destruction", verifyThread_->threadName);
		}
		verifyThread_->deleteLater();
	}
}

void ThreadVerify::stopVerifyThread()
{
	if (ownerOfVerifyThread_ && verifyThread_->isRunning()) {
		shutdownThread();
		verifyThread_->wait();
	}
}

void ThreadVerify::execCall(const impl::Functor * func)
{
	if (!verifyThread_->threadObject) {
		logWarn("execCall for already terminated thread %1", verifyThread_->threadName);
		return;
	}
	QCoreApplication::postEvent(verifyThread_->threadObject, new impl::ThreadHolderEvent(func));
}

void ThreadVerify::shutdownThread()
{
	if (!verifyThreadCall(&ThreadVerify::shutdownThread)) return;
	logFunctionTrace
	deleteThreadData();
	verifyThread_->quit();
}

}}	// namespace

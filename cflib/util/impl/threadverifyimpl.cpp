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

#include "threadverifyimpl.h"

#include <cflib/libev/libev.h>
#include <cflib/util/log.h>

USE_LOG(LogCat::Etc)

namespace cflib { namespace util { namespace impl {

bool ThreadObject::event(QEvent * event)
{
	if (event->type() == QEvent::User) {
		const Functor * func = ((ThreadHolderEvent *)event)->func;
		(*func)();
		delete func;
		return true;
	}
	return QObject::event(event);
}

ThreadHolder::ThreadHolder(const QString & threadName) :
	threadName(threadName), isActive_(true)
{
}

ThreadHolderQt::ThreadHolderQt(const QString & threadName) :
	ThreadHolder(threadName)
{
	logFunctionTrace
	threadObject_ = new ThreadObject();
	threadObject_->moveToThread(this);
	start();
}

bool ThreadHolderQt::doCall(const Functor * func)
{
	QCoreApplication::postEvent(threadObject_, new impl::ThreadHolderEvent(func));
	return true;
}

void ThreadHolderQt::stopLoop()
{
	quit();
}

void ThreadHolderQt::run()
{
	logDebug("thread %1 started with Qt event loop", threadName);
	exec();
	isActive_ = false;
	logDebug("thread %1 events stopped", threadName);
	delete threadObject_;
	threadObject_ = 0;
	logDebug("thread %1 stopped", threadName);
}

ThreadHolderLibEV::ThreadHolderLibEV(const QString & threadName, bool isWorkerOnly) :
	ThreadHolder(threadName),
	loop_(ev_loop_new(EVFLAG_NOSIGMASK | (isWorkerOnly ? EVBACKEND_SELECT : EVBACKEND_ALL))),
	wakeupWatcher_(new ev_async)
{
	logFunctionTrace
	ev_async_init(wakeupWatcher_, &ThreadHolderLibEV::asyncCallback);
	wakeupWatcher_->data = this;
    ev_async_start(loop_, wakeupWatcher_);
}

ThreadHolderLibEV::~ThreadHolderLibEV()
{
	logFunctionTrace
	ev_async_stop(loop_, wakeupWatcher_);
	wakeupWatcher_->data = 0;
	delete wakeupWatcher_;
	ev_loop_destroy(loop_);
}

void ThreadHolderLibEV::stopLoop()
{
	ev_break(loop_, EVBREAK_ALL);
}

void ThreadHolderLibEV::wakeUp()
{
	ev_async_send(loop_, wakeupWatcher_);
}

void ThreadHolderLibEV::run()
{
	logDebug("thread %1 started with libev backend %2", threadName, ev_backend(loop_));
	ev_run(loop_, 0);
	isActive_ = false;
	logDebug("thread %1 stopped", threadName);
}

void ThreadHolderLibEV::asyncCallback(ev_loop *, ev_async * w, int)
{
	((ThreadHolderLibEV *)w->data)->wokeUp();
}

ThreadHolderWorkerPool::ThreadHolderWorkerPool(const QString & threadName, bool isWorkerOnly, uint threadCount)
:
	ThreadHolderLibEV(QString("%1 1/%2").arg(threadName).arg(threadCount), isWorkerOnly),
	externalCalls_(1024),
	stopLoop_(false)
{
	logFunctionTrace

	for (uint i = 2 ; i <= threadCount ; ++i) {
		Worker * thread = new Worker(QString("%1 %2/%3").arg(threadName).arg(i).arg(threadCount), externalCalls_);
		workers_ << thread;
	}

    start();
}

ThreadHolderWorkerPool::~ThreadHolderWorkerPool()
{
	foreach (Worker * w, workers_) delete w;
}

bool ThreadHolderWorkerPool::doCall(const Functor * func)
{
	if (!externalCalls_.put(func)) return false;
	wakeUp();
	foreach (Worker * w, workers_) w->wakeUp();
	return true;
}

void ThreadHolderWorkerPool::stopLoop()
{
	logFunctionTrace
	stopLoop_ = true;
	wakeUp();
	foreach (Worker * w, workers_) w->stopLoop();
}

void ThreadHolderWorkerPool::wokeUp()
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

void ThreadHolderWorkerPool::run()
{
	logFunctionTrace
	ThreadHolderLibEV::run();
	foreach (Worker * w, workers_) w->wait();
}

ThreadHolderWorkerPool::Worker::Worker(const QString & threadName, ThreadFifo<const Functor> & externalCalls)
:
	ThreadHolderLibEV(threadName, true),
	externalCalls_(externalCalls),
	stopLoop_(false)
{
    start();
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

}}}	// namespace

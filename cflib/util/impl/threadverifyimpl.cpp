/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "threadverifyimpl.h"

#include <cflib/util/libev.h>
#include <cflib/util/log.h>
#include <cflib/util/threadstats.h>

USE_LOG(LogCat::Etc)

namespace cflib { namespace util { namespace impl {

ThreadObject::ThreadObject(int threadId, ThreadStats * stats) :
	threadId_(threadId), stats_(stats)
{
}

bool ThreadObject::event(QEvent * event)
{
	if (event->type() == QEvent::User) {
		QElapsedTimer elapsed;
		if (stats_) elapsed.start();
		const Functor * func = ((ThreadHolderEvent *)event)->func;
		(*func)();
		delete func;
		if (stats_) stats_->externNewCallTime(threadId_, elapsed.nsecsElapsed());
		return true;
	}
	return QObject::event(event);
}

ThreadHolder::ThreadHolder(const QString & threadName, int threadId, ThreadStats * stats, bool disable) :
	threadName(threadName),
	threadId_(threadId), stats_(stats),
	disabled_(disable), isActive_(true)
{
	setObjectName(threadName);
}

ThreadHolder::~ThreadHolder()
{
	logTrace("~ThreadHolder()");
}

ThreadHolderQt::ThreadHolderQt(const QString & threadName, int threadId, ThreadStats * stats, bool disable) :
	ThreadHolder(threadName, threadId, stats, disable)
{
	threadObject_ = new ThreadObject(threadId, stats);
	if (!disable) {
		threadObject_->moveToThread(this);
		start();
	}
}

bool ThreadHolderQt::doCall(const Functor * func)
{
	QCoreApplication::postEvent(threadObject_, new impl::ThreadHolderEvent(func));
	return true;
}

void ThreadHolderQt::execLater(const Functor * func) const
{
	QCoreApplication::postEvent(threadObject_, new impl::ThreadHolderEvent(func));
}

void ThreadHolderQt::stopLoop()
{
	if (!disabled_) {
		quit();
	} else {
		isActive_ = false;
		delete threadObject_;
		threadObject_ = 0;
	}
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

ThreadHolderLibEV::ThreadHolderLibEV(const QString & threadName, int threadId, ThreadStats * stats, bool isWorkerOnly, bool disable) :
	ThreadHolder(threadName, threadId, stats, disable),
	loop_(ev_loop_new(EVFLAG_NOSIGMASK | (isWorkerOnly ? EVBACKEND_SELECT : EVBACKEND_ALL))),
	wakeupWatcher_(new ev_async)
{
	ev_async_init(wakeupWatcher_, &ThreadHolderLibEV::asyncCallback);
	wakeupWatcher_->data = this;
	ev_async_start(loop_, wakeupWatcher_);
}

ThreadHolderLibEV::~ThreadHolderLibEV()
{
	ev_async_stop(loop_, wakeupWatcher_);
	wakeupWatcher_->data = 0;
	delete wakeupWatcher_;
	ev_loop_destroy(loop_);
}

void ThreadHolderLibEV::stopLoop()
{
	ev_break(loop_, EVBREAK_ALL);
}

void ThreadHolderLibEV::execLater(const Functor * func) const
{
	ev_once(loop_, -1, 0, 0.0, &ThreadHolderLibEV::execLaterCall, (void *)func);
}

void ThreadHolderLibEV::execLaterCall(int, void * arg)
{
	const Functor * func = (const Functor *)arg;
	(*func)();
	delete func;
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

ThreadHolderWorkerPool::ThreadHolderWorkerPool(const QString & threadName,
	int threadId, ThreadStats * stats, bool isWorkerOnly, uint threadCount)
:
	ThreadHolderLibEV(threadCount > 1 ? QString("%1 1/%2").arg(threadName).arg(threadCount) : threadName,
		threadId, stats, isWorkerOnly, threadCount == 0),
	externalCalls_(1024),
	stopLoop_(false)
{
	if (!disabled_) start();
	for (uint i = 2 ; i <= threadCount ; ++i) {
		Worker * thread = new Worker(QString("%1 %2/%3").arg(threadName).arg(i).arg(threadCount), threadId, stats, i - 1, externalCalls_);
		workers_ << thread;
	}
}

ThreadHolderWorkerPool::~ThreadHolderWorkerPool()
{
	foreach (Worker * w, workers_) delete w;
}

bool ThreadHolderWorkerPool::doCall(const Functor * func)
{
	if (!externalCalls_.put(func)) {
		if (stats_) stats_->externOverflow(threadId_);
		const impl::ThreadHolder * thread = dynamic_cast<const impl::ThreadHolder *>(QThread::currentThread());
		logWarn("queue of thread %1 full (called by %2)", threadName, thread ? thread->threadName : "?");
		return false;
	}
	wakeUp();
	foreach (Worker * w, workers_) w->wakeUp();
	return true;
}

void ThreadHolderWorkerPool::stopLoop()
{
	if (!disabled_) {
		stopLoop_ = true;
		wakeUp();
		foreach (Worker * w, workers_) w->stopLoop();
	} else {
		isActive_ = false;
	}
}

bool ThreadHolderWorkerPool::isOwnThread() const
{
	QThread * current = QThread::currentThread();
	if (current == this || disabled_) return true;
	foreach (QThread * w, workers_) if (current == w) return true;
	return false;
}

uint ThreadHolderWorkerPool::threadCount() const
{
	return 1 + workers_.size();
}

void ThreadHolderWorkerPool::wokeUp()
{
	if (stopLoop_) {
		ThreadHolderLibEV::stopLoop();
		return;
	}

	QElapsedTimer elapsed;
	if (stats_) elapsed.start();
	while (const Functor * func = externalCalls_.take()) {
		(*func)();
		delete func;
	}
	if (stats_) stats_->externNewCallTime(threadId_, elapsed.nsecsElapsed());
}

void ThreadHolderWorkerPool::run()
{
	ThreadHolderLibEV::run();
	foreach (Worker * w, workers_) w->wait();
}

ThreadHolderWorkerPool::Worker::Worker(const QString & threadName,
	int threadId, ThreadStats * stats, uint threadNo, ThreadFifo<const Functor *> & externalCalls)
:
	ThreadHolderLibEV(threadName, threadId, stats, true, false),
	threadNo_(threadNo),
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

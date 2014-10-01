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
	threadObject = new ThreadObject();
	threadObject->moveToThread(this);
}

ThreadHolder::ThreadHolder(const QString & threadName, ThreadObject * threadObject) :
	threadName(threadName), threadObject(threadObject), isActive_(true)
{
}

void ThreadHolder::run()
{
	logDebug("thread %1 started with Qt event loop", threadName);
	exec();
	isActive_ = false;
	logDebug("thread %1 events stopped", threadName);
	delete threadObject;
	threadObject = 0;
	logDebug("thread %1 stopped", threadName);
}

ThreadHolderLibEV::ThreadHolderLibEV(const QString & threadName) :
	ThreadHolder(threadName, 0),
	loop_(ev_loop_new(EVFLAG_NOSIGMASK | EVBACKEND_ALL)),
	wakeupWatcher_(new ev_async),
	externalCalls_(1024)
{
	ev_async_init(wakeupWatcher_, &ThreadHolderLibEV::wakeup);
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

bool ThreadHolderLibEV::doCall(const Functor * func)
{
	if (!externalCalls_.put(func)) return false;
	ev_async_send(loop_, wakeupWatcher_);
	return true;
}

void ThreadHolderLibEV::run()
{
	logDebug("thread %1 started with libev backend %2", threadName, ev_backend(loop_));

	while (const Functor * func = externalCalls_.take()) {
		(*func)();
		delete func;
	}

	ev_run(loop_, 0);

	isActive_ = false;
	logDebug("thread %1 stopped", threadName);
}

void ThreadHolderLibEV::wakeup(ev_loop *, ev_async * w, int)
{
	ThreadHolderLibEV * th = (ThreadHolderLibEV *)w->data;
	while (const Functor * func = th->externalCalls_.take()) {
		(*func)();
		delete func;
	}
}

}}}	// namespace

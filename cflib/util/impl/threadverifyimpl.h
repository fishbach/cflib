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

#pragma once

#include <cflib/util/functor.h>
#include <cflib/util/threadfifo.h>

struct ev_async;
struct ev_loop;

namespace cflib { namespace util { namespace impl {

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
	virtual bool event(QEvent * event);
};

class ThreadHolder : public QThread
{
public:
	ThreadHolder(const QString & threadName);

	bool isActive() const { return isActive_; }
	const QString threadName;
	ThreadObject * threadObject;

protected:
	ThreadHolder(const QString & threadName, ThreadObject * threadObject);
    virtual void run();

protected:
	bool isActive_;
};

class ThreadHolderLibEV : public ThreadHolder
{
public:
	ThreadHolderLibEV(const QString & threadName);
	~ThreadHolderLibEV();

	ev_loop * loop() const { return loop_; }
	void stopLoop();
	bool doCall(const Functor * func);

protected:
    virtual void run();

private:
	static void wakeup(ev_loop * loop, ev_async * w, int revents);

private:
	ev_loop * loop_;
	ev_async * wakeupWatcher_;
	ThreadFifo<const Functor *> externalCalls_;
};

}}}	// namespace

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
	ThreadHolder(const QString & threadName, bool disable);
	~ThreadHolder();

	const QString threadName;
	bool isActive() const { return isActive_; }
	virtual bool doCall(const Functor * func) = 0;
	virtual void stopLoop() = 0;
	virtual bool isOwnThread() const { return QThread::currentThread() == this || disabled_; }
	virtual uint threadCount() const { return 1; }
	virtual uint threadNo() const    { return 0; }

protected:
	const bool disabled_;
	bool isActive_;
};

class ThreadHolderQt : public ThreadHolder
{
public:
	ThreadHolderQt(const QString & threadName, bool disable);

	ThreadObject * threadObject() const { return threadObject_; }
	virtual bool doCall(const Functor * func);
	virtual void stopLoop();

protected:
	virtual void run();

private:
	ThreadObject * threadObject_;
};

class ThreadHolderLibEV : public ThreadHolder
{
public:
	virtual ~ThreadHolderLibEV();

	virtual void stopLoop();
	ev_loop * loop() const { return loop_; }
	void wakeUp();

protected:
	ThreadHolderLibEV(const QString & threadName, bool isWorkerOnly, bool disable);
	virtual void run();
	virtual void wokeUp() = 0;

private:
	static void asyncCallback(ev_loop * loop, ev_async * w, int revents);

private:
	ev_loop * loop_;
	ev_async * wakeupWatcher_;
};

class ThreadHolderWorkerPool : public ThreadHolderLibEV
{
public:
	ThreadHolderWorkerPool(const QString & threadName, bool isWorkerOnly, uint threadCount);
	~ThreadHolderWorkerPool();

	virtual bool doCall(const Functor * func);
	virtual void stopLoop();
	virtual bool isOwnThread() const;
	virtual uint threadCount() const;

protected:
	virtual void run();
	virtual void wokeUp();

private:
	class Worker : public ThreadHolderLibEV
	{
	public:
		Worker(const QString & threadName, uint threadNo, ThreadFifo<const Functor *> & externalCalls);

		virtual bool doCall(const Functor *) { return false; }
		virtual void stopLoop();
		virtual uint threadNo() const { return threadNo_; }

	protected:
		virtual void wokeUp();

	private:
		const uint threadNo_;
		ThreadFifo<const Functor *> & externalCalls_;
		bool stopLoop_;
	};

	ThreadFifo<const Functor *> externalCalls_;
	QList<Worker *> workers_;
	bool stopLoop_;
};

}}}	// namespace

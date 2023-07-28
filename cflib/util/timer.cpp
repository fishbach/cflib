/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "timer.h"

#include <cflib/util/libev.h>
#include <cflib/util/threadverify.h>

namespace cflib { namespace util {

namespace {

class TimerObject : public QObject
{
	Q_OBJECT
public:
	TimerObject(const Functor * func) : func_(func) {}

public slots:
	void timeout()
	{
		(*func_)();
		delete func_;
		delete this;
	}

private:
	const Functor * func_;
};

}

void Timer::singleShot(double afterSecs, const Functor * func)
{
	ev_loop * loop = libEVLoopOfThread();
	if (loop) ev_once(loop, -1, 0, afterSecs, &Timer::timeout, (void *)func);
	else      QTimer::singleShot(afterSecs * 1000, new TimerObject(func), SLOT(timeout()));
}

void Timer::timeout(int, void * arg)
{
	const Functor * func = (const Functor *)arg;
	(*func)();
	delete func;
}

}}	// namespace

#include "timer.moc"

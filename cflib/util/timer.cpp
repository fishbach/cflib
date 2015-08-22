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
		deleteLater();
	}

private:
	const Functor * func_;
};

}

void Timer::singleShot(double after, const Functor * func)
{
	ev_loop * loop = libEVLoopOfThread();
	if (loop) ev_once(loop, -1, 0, after, &Timer::timeout, (void *)func);
	else      QTimer::singleShot(after * 1000, new TimerObject(func), SLOT(timeout()));
}

void Timer::timeout(int, void * arg)
{
	const Functor * func = (const Functor *)arg;
	(*func)();
	delete func;
}

}}	// namespace

#include "timer.moc"

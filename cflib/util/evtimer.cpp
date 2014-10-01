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

#include "evtimer.h"

#include <cflib/libev/libev.h>
#include <cflib/util/log.h>

USE_LOG(LogCat::Etc)

namespace cflib { namespace util {

void EVTimer::init()
{
	if (!QThread::currentThread()->property("isLibEV").toBool()) {
		logWarn("current thread does not have a libev event loop");
		return;
	}

	loop_ = ((const impl::ThreadHolderLibEV *)QThread::currentThread())->loop();
	timer_ = new ev_timer;
	ev_init(timer_, &EVTimer::timeout);
	timer_->data = this;
}

EVTimer::~EVTimer()
{
	ev_timer_stop(loop_, timer_);
	timer_->data = 0;
	delete timer_;
	delete func_;
}

void EVTimer::start(double after, double repeat)
{
	ev_timer_set(timer_, after, repeat);
	ev_timer_start(loop_, timer_);
}

void EVTimer::stop()
{
	ev_timer_stop(loop_, timer_);
}

void EVTimer::timeout(ev_loop *, ev_timer * w, int)
{
	EVTimer * timer = (EVTimer *)w->data;
	(*timer->func_)();
}

}}	// namespace

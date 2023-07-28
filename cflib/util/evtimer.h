/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/util/functor.h>

struct ev_loop;
struct ev_timer;

namespace cflib { namespace util {

class EVTimer
{
public:
	template<typename C>
	EVTimer(C * obj, void (C::*method)()) :
		timer_(0),
		func_(new Functor0<C>(obj, method))
	{
		init();
	}
	~EVTimer();

	void start(double after, double repeat);
	void start(double repeat) { start(repeat, repeat); }
	void stop();

private:
	void init();
	static void timeout(ev_loop * loop, ev_timer * w, int revents);

private:
	ev_timer * timer_;
	Functor * func_;
};

}}	// namespace

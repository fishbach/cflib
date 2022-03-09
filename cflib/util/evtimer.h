/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
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

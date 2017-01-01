/* Copyright (C) 2013-2016 Christian Fischbach <cf@cflib.de>
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

namespace cflib { namespace util {

class Timer
{
public:
	static void singleShot(double afterSecs, const Functor * func);

	template<typename C>
	static void singleShot(double afterSecs, C * obj, void (C::*func)())
	{
		singleShot(afterSecs, new util::Functor0<C>(obj, func));
	}

	template<typename C>
	static void singleShot(double afterSecs, const C * obj, void (C::*func)() const)
	{
		singleShot(afterSecs, new util::Functor0C<C>(obj, func));
	}

private:
	static void timeout(int revents, void * arg);
};

}}	// namespace

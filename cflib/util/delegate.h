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

#include <utility>

namespace cflib { namespace util {

template<typename C, typename R, typename... P> class Delegate;

template<typename C, typename R, typename... P>
class Delegate<C *, R, P...>
{
public:
	Delegate(C * obj, R (C::*func)(P...)) : obj_(obj), func_(func) {}

	template<typename... A>
	inline R operator()(P... p, A...) const
	{
		return (obj_->*func_)(std::forward<P>(p)...);
	}

private:
	C * obj_;
	R (C::*func_)(P...);
};

template<typename C, typename R, typename... P>
class Delegate<const C *, R, P...>
{
public:
	Delegate(const C * obj, R (C::*func)(P...) const) : obj_(obj), func_(func) {}

	template<typename... A>
	inline R operator()(P... p, A...) const
	{
		return (obj_->*func_)(std::forward<P>(p)...);
	}

private:
	const C * obj_;
	R (C::*func_)(P...) const;
};

template<typename... P, typename C, typename R>
inline Delegate<C *, R, P...> delegate(C * obj, R (C::*func)(P...))
{
	return Delegate<C *, R, P...>(obj, func);
}

template<typename... P, typename C, typename R>
inline Delegate<const C *, R, P...> delegate(const C * obj, R (C::*func)(P...) const)
{
	return Delegate<const C *, R, P...>(obj, func);
}

}}	// namespace

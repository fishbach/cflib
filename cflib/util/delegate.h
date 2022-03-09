/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
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

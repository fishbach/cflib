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

#include <cflib/util/delegate.h>

#include <functional>

#define cfsignals \
	public: \
		template<typename F> class sig : public cflib::util::Sig<F> {}; \
	public

namespace cflib { namespace util {

template<typename> class Sig;

template<typename... P>
class Sig<void (P...)>
{
public:
	typedef std::function<void (P...)> Func;

public:
	template<typename F>
	void bind(F func)
	{
		listeners_.push_back(func);
	}

	template<typename... A, typename C>
	void bind(C * obj, void (C::*func)(A...))
	{
		listeners_.push_back(Delegate<C *, void, A...>(obj, func));
	}

	template<typename... A, typename C>
	void bind(const C * obj, void (C::*func)(A...) const)
	{
		listeners_.push_back(Delegate<const C *, void, A...>(obj, func));
	}

	void unbindAll()
	{
		listeners_.clear();
	}

	template<typename... A>
	inline void operator()(P... p, A...) const
	{
		for (auto it = listeners_.begin() ; it != listeners_.end() ; ++it) (*it)(std::forward<P>(p)...);
	}

private:
	std::vector<Func> listeners_;
};

template<typename R, typename... P>
class Sig<R (P...)>
{
public:
	typedef std::function<R (P...)> Func;

public:
	template<typename F>
	void bind(F func)
	{
		listener_ = func;
	}

	template<typename... A, typename C>
	void bind(C * obj, R (C::*func)(A...))
	{
		listener_ = Delegate<C *, R, A...>(obj, func);
	}

	template<typename... A, typename C>
	void bind(const C * obj, R (C::*func)(A...) const)
	{
		listener_ = Delegate<const C *, R, A...>(obj, func);
	}

	void unbind()
	{
		listener_ = Func();
	}

	template<typename... A>
	inline R operator()(P... p, A...) const
	{
		return listener_(std::forward<P>(p)...);
	}

private:
	Func listener_;
};

}}	// namespace

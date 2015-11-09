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

#pragma once

#include <QtCore>

#include <functional>

#define cfsignals \
	public: \
		template<typename F> class  sig : public cflib::util::Sig<F> {}; \
		template<typename F> class rsig : public cflib::util::Sig<F> {}; \
	public

namespace cflib { namespace util {

template<typename> class Sig;

template<typename... P>
class Sig<void (P...)>
{
public:
	typedef std::function<void (P...)> Func;

public:
	void bind(const Func & func)
	{
		listeners_.push_back(func);
	}

	template<typename C>
	void bind(C * obj, void (C::*func)(P...))
	{
		listeners_.push_back([obj, func](P... p) { (obj->*func)(std::forward<P>(p)...); });
	}

	template<typename C>
	void bind(const C * obj, void (C::*func)(P...) const)
	{
		listeners_.push_back([obj, func](P... p) { (obj->*func)(std::forward<P>(p)...); });
	}

	void bind(Sig<void (P...)> & sig)
	{
		listeners_.push_back(sig);
	}

	void unbindAll()
	{
		listeners_.clear();
	}

	void operator()(P... p) const
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
	void bind(const Func & func)
	{
		listener_ = func;
	}

	template<typename C>
	void bind(C * obj, R (C::*func)(P...))
	{
		listener_ = [obj, func](P... p) { return (obj->*func)(std::forward<P>(p)...); };
	}

	template<typename C>
	void bind(const C * obj, R (C::*func)(P...) const)
	{
		listener_ = [obj, func](P... p) { return (obj->*func)(std::forward<P>(p)...); };
	}

	void bind(Sig<R (P...)> & sig)
	{
		listener_ = sig;
	}

	void unbind()
	{
		listener_ = Func();
	}

	R operator()(P... p) const
	{
		return listener_(std::forward<P>(p)...);
	}

private:
	Func listener_;
};

}}	// namespace

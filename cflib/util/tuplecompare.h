/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <tuple>

namespace cflib { namespace util {

namespace impl {

template <size_t I, typename... P> struct TupleCmp;

template <size_t I, typename P, typename... PN>
struct TupleCmp<I, P, PN...>
{
	template <typename T>
	bool operator()(const T & t, size_t count, P p, PN... pn)
	{
		return I >= count || (std::get<I>(t) == p && TupleCmp<I + 1, PN...>()(t, count, pn...));
	}
};

template <size_t I>
struct TupleCmp<I>
{
	template <typename T>
	bool operator()(const T &, size_t)
	{
		return true;
	}
};

template <size_t I, size_t S, typename R>
struct TupleCall
{
	template <typename F, typename... T, typename... P>
	R operator()(F func, const std::tuple<T...> & tp, P&&... p)
	{
		return TupleCall<I + 1, S, R>()(func, tp, std::forward<P>(p)..., std::get<I>(tp));
	}

	template <typename F, typename... T, typename... P>
	R operator()(F func, std::tuple<T...> & tp, P&&... p)
	{
		return TupleCall<I + 1, S, R>()(func, tp, std::forward<P>(p)..., std::get<I>(tp));
	}
};

template <size_t S, typename R>
struct TupleCall<S, S, R>
{
	template <typename F, typename... T, typename... P>
	R operator()(F func, const std::tuple<T...> &, P&&... p)
	{
		return func(std::forward<P>(p)...);
	}

	template <typename F, typename... T, typename... P>
	R operator()(F func, std::tuple<T...> &, P&&... p)
	{
		return func(std::forward<P>(p)...);
	}
};

}

template <typename... TP, typename... P>
inline bool equal(const std::tuple<TP...> & t, P... p)
{
    return impl::TupleCmp<0, P...>()(t, sizeof...(TP), p...);
}

template <typename... TP, typename... P>
inline bool partialEqual(const std::tuple<TP...> & t, size_t count, P... p)
{
    return impl::TupleCmp<0, P...>()(t, count, p...);
}

template <typename R, typename F, typename... T, typename... P>
inline R callWithTupleParams(F func, const std::tuple<T...> & tp, P&&... p)
{
	return impl::TupleCall<0, sizeof...(T), R>()(func, tp, std::forward<P>(p)...);
}

template <typename R, typename F, typename... T, typename... P>
inline R callWithTupleParams(F func, std::tuple<T...> & tp, P&&... p)
{
	return impl::TupleCall<0, sizeof...(T), R>()(func, tp, std::forward<P>(p)...);
}

}}	// namespace

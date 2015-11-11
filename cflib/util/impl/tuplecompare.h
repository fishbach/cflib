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

#include <tuple>

namespace cflib { namespace util { namespace impl {

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

template <typename... TP, typename... P>
inline bool equal(const std::tuple<TP...> & t, P... p)
{
    return TupleCmp<0, P...>()(t, sizeof...(TP), p...);
}

template <typename... TP, typename... P>
inline bool partialEqual(const std::tuple<TP...> & t, size_t count, P... p)
{
    return TupleCmp<0, P...>()(t, count, p...);
}

}}}	// namespace

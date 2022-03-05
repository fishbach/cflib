/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
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

#include <cflib/serialize/serializeber.h>

namespace cflib { namespace serialize { namespace impl {

template <size_t I, typename... P> struct SomeToByteArray;

template <size_t I, typename P, typename... PN>
struct SomeToByteArray<I, P, PN...>
{
	void operator()(BERSerializer & ser, size_t start, size_t count, P p, PN... pn)
	{
		if (I >= start + count) return;
		if (I < start) ser << Placeholder();
		else           ser << p;
		SomeToByteArray<I + 1, PN...>()(ser, start, count, pn...);
	}
};

template <size_t I>
struct SomeToByteArray<I>
{
	void operator()(BERSerializer &, size_t, size_t) {}
};

template <typename... P> struct ToByteArray;

template <typename P, typename... PN>
struct ToByteArray<P, PN...>
{
	void operator()(BERSerializer & ser, P p, PN... pn)
	{
		ser << p;
		ToByteArray<PN...>()(ser, std::forward<PN>(pn)...);
	}
};

template <>
struct ToByteArray<>
{
	void operator()(BERSerializer &) {}
};

}}}	// namespace

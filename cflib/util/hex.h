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

namespace cflib { namespace util {

// 0 .. 15 -> '0' .. '9', 'A' .. 'F'
inline char toHex(quint8 b)
{
	if (b >= 10) return b + 55;
	else return b + 48;
}

inline quint8 fromHex(char c)
{
	if      (c >= 'a') c -= 'a' - 10;
	else if (c >= 'A') c -= 'A' - 10;
	else               c -= '0';
	return (quint8)c;
}

}}	// namespace

/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
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

inline QByteArray uint64ToHex(quint64 n)
{
	QByteArray rv(16, '0');
	uint p = 16;
	while (n > 0) {
		rv[--p] = toHex(n & 0xF);
		n >>= 4;
	}
	return rv;
}

}}	// namespace

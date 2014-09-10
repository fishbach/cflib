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

#include <cflib/serialize/serializeber.h>

namespace cflib { namespace serialize {

// returns the length of the complete BER packet (Tag size + Length size + Value size)
// returns -1 if not enough data available
// returns -2 if length is undefined (one byte: 0x80)
// returns -3 if too big length was found (max TLV size is 2^32 - 1)
inline qint32 getTLVLength(const QByteArray & data)
{
	int lengthSize;
	const qint64 length = impl::decodeBERLength((const quint8 *)data.constData() + 1, data.size() - 1, lengthSize);
	if (length < 0) return length;
	if (length > Q_INT64_C(0x7FFFFFFE) - lengthSize) return -3;
	const qint32 retval = 1 + lengthSize + (qint32)length;
	if (retval > data.size()) return -1;
	return retval;
}

template<typename T>
inline QByteArray toByteArray(const T & v) {
	BERSerializer s; s << v;
	return s.data();
}

template<typename T>
inline T fromByteArray(const QByteArray & ba) {
	T retval;
	BERDeserializer(ba) >> retval;
	return retval;
}

}}	// namespace

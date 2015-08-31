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
inline qint32 getTLVLength(const QByteArray & data, quint64 & tag, int & tagLen, int & lengthSize)
{
	quint64 tagBytes;
	const qint64 valueLen = impl::decodeTLV((const quint8 *)data.constData(), data.size(), tagBytes, tagLen, lengthSize);
	if (valueLen < 0) return valueLen;
	if (valueLen > Q_INT64_C(0x7FFFFFFF) - tagLen - lengthSize) return -3;
	const qint64 rv = tagLen + lengthSize + valueLen;
	if (data.size() < rv) return -1;
	tag = impl::getTagNumber(tagBytes, tagLen);
	return rv;
}

template<typename T>
inline QByteArray toByteArray(const T & v, quint64 tag = 1)
{
	quint8 tagLen;
	quint64 tagBytes = impl::createTag(tag, tagLen);
	QByteArray rv;
	impl::BERSerializerBase * dummy = 0;
	impl::serializeBER(v, tagBytes, tagLen, rv, *dummy);
	return rv;
}

template<typename T>
inline T fromByteArray(const QByteArray & data, qint32 tlvLen, int tagLen, int lengthSize)
{
	qint32 valueLen = tlvLen - tagLen - lengthSize;
	if (valueLen == 0 && lengthSize == 2) return T();
	T retval;
	impl::BERDeserializerBase * dummy = 0;
	impl::deserializeBER(retval, (const quint8 *)data.constData() + tagLen + lengthSize, valueLen, *dummy);
	return retval;
}

template<typename T>
inline T fromByteArray(const QByteArray & data)
{
	quint64 tag;
	int tagLen;
	int lengthSize;
	const qint64 valueLen = impl::decodeTLV((const quint8 *)data.constData(), data.size(), tag, tagLen, lengthSize);
	if (valueLen < 0 || valueLen > Q_INT64_C(0x7FFFFFFF) - tagLen - lengthSize) return T();

	if (valueLen == 0 && lengthSize == 2) return T();
	T retval;
	impl::BERDeserializerBase * dummy = 0;
	impl::deserializeBER(retval, (const quint8 *)data.constData() + tagLen + lengthSize, valueLen, *dummy);
	return retval;
}

}}	// namespace

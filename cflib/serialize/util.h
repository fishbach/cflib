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

#include <cflib/serialize/impl/readandcall.h>
#include <cflib/serialize/impl/sometobytearray.h>

namespace cflib { namespace serialize {

// returns the value length of the BER TLV
// returns -1 if not enough data available
// returns -2 if length is undefined (one byte: 0x80)
// returns -3 if too big length was found (max TLV size is 2^32 - 1)
inline qint32 getTLVLength(const QByteArray & data, quint64 & tagNo, int & tagLen, int & lengthSize)
{
	const qint64 valueLen = impl::decodeTLV((const quint8 *)data.constData(), data.size(), tagNo, tagLen, lengthSize);
	if (valueLen < 0) return valueLen;
	if (valueLen > Q_INT64_C(0x7FFFFFFF) - tagLen - lengthSize) return -3;
	if (data.size() < tagLen + lengthSize + valueLen) return -1;
	return valueLen;
}

inline QByteArray emptyTag(quint64 tagNo)
{
	const quint8 tagLen = impl::calcTagLen(tagNo);
	QByteArray rv(tagLen + 1, '\0');
	impl::writeTagBytes((quint8 *)rv.constData(), tagNo, false, tagLen);
	return rv;
}

template<typename T>
inline QByteArray toByteArray(const T & v, quint64 tagNo = 1)
{
	QByteArray rv;
	impl::BERSerializerBase * dummy = 0;
	impl::serializeBER(v, tagNo, rv, *dummy);
	return rv;
}

template<typename... P>
inline void toByteArray(BERSerializer & ser, P... p)
{
	impl::ToByteArray<P...>()(ser, std::forward<P>(p)...);
}

template<typename... P>
inline void someToByteArray(BERSerializer & ser, uint start, uint count, P... p)
{
	impl::SomeToByteArray<0, P...>()(ser, start, count, std::forward<P>(p)...);
}

template<typename T>
inline T fromByteArray(const QByteArray & data, int tagLen, int lengthSize, qint32 valueLen)
{
	if (valueLen == 0 && lengthSize == 2) return T();
	T retval;
	impl::BERDeserializerBase * dummy = 0;
	impl::deserializeBER(retval, (const quint8 *)data.constData() + tagLen + lengthSize, valueLen, *dummy);
	return retval;
}

template<typename T>
inline T fromByteArray(const QByteArray & data)
{
	quint64 tag = 0;
	int tagLen = 0;
	int lengthSize = 0;
	const qint64 valueLen = getTLVLength(data, tag, tagLen, lengthSize);
	if (valueLen < 0) return T();

	if (valueLen == 0 && lengthSize == 2) return T();
	T retval;
	impl::BERDeserializerBase * dummy = 0;
	impl::deserializeBER(retval, (const quint8 *)data.constData() + tagLen + lengthSize, valueLen, *dummy);
	return retval;
}

template<typename... P, typename F>
inline void readAndCall(BERDeserializer & deser, F func)
{
	impl::ReadAndCall<P...>()(deser, func);
}

}}	// namespace

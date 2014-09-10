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

namespace cflib { namespace serialize { namespace impl {

inline bool isZeroTag(quint64 tag, quint8 tagLen)
{
	return (tag & 0x1f) == 0 && tagLen == 1;
}

inline quint64 setConstructedBit(quint64 tag, quint8 tagLen)
{
	return tag | (0x20 << ((tagLen-1) * 8));
}

inline void incTag(quint64 & tag, quint8 & tagLen)
{
	if (isZeroTag(tag, tagLen)) return;
	++tag;
	if (tagLen == 1) {
		if ((tag & 0x1F) == 0x1F) {
			tag <<= 8;
			tag |= 0x1F;
			++tagLen;
		}
		return;
	}
	if ((tag & 0x80) == 0) return;

	int c = 1;
	for ( ; c < tagLen - 1 ; ++c) {
		quint8 b = tag >> (c * 8);
		if (b != 0xff) {
			tag += Q_UINT64_C(0x01) << (c * 8);
			tag &= ~Q_UINT64_C(0xff);
			return;
		}
		tag &= ~(Q_UINT64_C(0x7f) << (c * 8));
	}

	// one byte more
	tag <<= 8;
	tag |= Q_UINT64_C(0x01) << (c * 8);
	++tagLen;
}

inline quint8 minSizeOfUInt(quint64 v)
{
	if (v < 256) return 1;
	// logarithmic search
	uint l = 0;
	uint h = 64;
	while (h - l > 8) {
		const uint t = (l + h) / 2;
		if (v >> t != 0) l = t;
		else             h = t;
	}
	return h / 8;
}

inline quint8 minSizeOfInt(qint64 v)
{
	if (v >= -128 && v <= 127) return 1;

	if (v > 0) {
		const quint64 uv = (quint64)v;
		const quint8 rv = minSizeOfUInt(uv);
		if (uv >> (rv * 8 - 1) == 0) return rv;
		return rv + 1;
	}

	uint l = 0;
	uint h = 64;
	while (h - l > 8) {
		const uint t = (l + h) / 2;
		if (v >> t != -1) l = t;
		else              h = t;
	}
	const quint8 rv = h / 8;
	if (v >> (rv * 8 - 1) == -1) return rv;
	return rv + 1;
}

// returns -1 if not enough data available
// returns -2 if length is undefined (one byte: 0x80)
// returns -3 if too big length was found
inline qint64 decodeBERLength(const quint8 * data, int len, int & lengthSize)
{
	// Is some data available?
	if (len < 1) {
		lengthSize = 0;
		return -1;
	}

	// If 8th bit is not set, length is in this byte.
	quint8 b = *data;
	if ((b & 0x80) == 0) {
		lengthSize = 1;
		return b;
	}

	// 8th bit is set, so lower bits hold the size of the length
	int ls = b & 0x7F;
	if (len <= ls || ls > 8) {
		lengthSize = 0;
		return ls > 8 ? -3 : -1;
	}

	// check for undefined length
	if (ls == 0) {
		lengthSize = 1;
		return -2;
	}

	// check for too big length (signed qint64 overflow)
	b = *(++data);
	if (ls == 8 && ((b & 0x80) != 0)) {
		lengthSize = 0;
		return -3;
	}

	// calculate length
	lengthSize = ls + 1;
	qint64 retval = b;
	while (--ls > 0) retval = (retval << 8) | *(++data);
	return retval;
}

// The first byte the length is returned. All other bytes are in lenBytes
// If length < 0 the undefined length is written.
inline quint8 encodeBERLength(qint64 length, QByteArray & lenBytes)
{
	// one byte encoding
	if (length < 0)    return 0x80;
	if (length < 0x80) return (quint8)length;

	// calculate needed bytes
	quint64 len = (quint64)length;
	quint8 size = minSizeOfUInt(len);
	const quint8 rv = size | 0x80;

	lenBytes.resize(size);
	quint8 * lb = (quint8 *)lenBytes.constData() + size;	// constData for performance

	// write length
	*(--lb) = (quint8)len;
	while (--size > 0) {
		len >>= 8;
		*(--lb) = (quint8)len;
	}

	return rv;
}

inline void insertBERLength(QByteArray & data, int oldSize)
{
	QByteArray lenBytes;
	data[oldSize - 1] = encodeBERLength(data.size() - oldSize, lenBytes);
	if (!lenBytes.isEmpty()) data.insert(oldSize, lenBytes);
}

inline void writeTag(quint8 * pos, quint64 tag, quint8 tagLen)
{
	pos += tagLen;
	*(--pos) = tag;
	while (--tagLen) {
		tag >>= 8;
		*(--pos) = tag;
	}
}

inline void writeTag(QByteArray & data, quint64 tag, quint8 tagLen)
{
	if (tagLen == 1) {
		data += (char)tag;
		return;
	}
	quint8 bytes[tagLen];
	writeTag(bytes, tag, tagLen);
	data.append((const char *)bytes, tagLen);
}

inline void writeNull(QByteArray & data, quint64 tag, quint8 tagLen)
{
	if (!isZeroTag(tag, tagLen)) return;

	quint8 bytes[tagLen + 1];
	writeTag(bytes, tag, tagLen);
	bytes[tagLen] = 0;
	data.append((const char *)bytes, tagLen + 1);
}

inline void writeZero(QByteArray & data, quint64 tag, quint8 tagLen)
{
	quint8 bytes[tagLen + 2];
	writeTag(bytes, tag, tagLen);
	bytes[tagLen] = 0x81;
	bytes[tagLen + 1] = 0;
	data.append((const char *)bytes, tagLen + 2);
}

class TLWriter
{
public:
	TLWriter(QByteArray & data, quint64 tag, quint8 tagLen) :
		data_(data)
	{
		writeTag(data, setConstructedBit(tag, tagLen), tagLen);
		data += '\0';
		oldSize_ = data.size();
	}

	~TLWriter()
	{
		insertBERLength(data_, oldSize_);
	}

private:
	QByteArray & data_;
	int oldSize_;
};

}}}	// namespace

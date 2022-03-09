/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <QtCore>

namespace cflib { namespace serialize { namespace impl {

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
inline qint64 decodeBERTag(const quint8 * data, int len, int & tagLen)
{
	if (len < 1) return -1;
	quint64 tagNo = *data & 0x1F;	// remove Class and P/C
	tagLen = 1;
	if (tagNo == 0x1F) {
		if (len < ++tagLen) return -1;
		quint8 b = *(++data);
		tagNo = b & 0x7F;
		while (b & 0x80) {
			if (len < ++tagLen) return -1;
			b = *(++data);
			tagNo = (tagNo << 7) | (b & 0x7F);
		}
	}
	return (qint64)tagNo;
}

// returns -1 if not enough data available
// returns -2 if length is undefined (one byte: 0x80)
// returns -3 if too big length was found
inline qint64 decodeBERLength(const quint8 * data, int len, int & lengthSize)
{
	// Is some data available?
	if (len < 1) return -1;

	// If 8th bit is not set, length is in this byte.
	quint8 b = *data;
	if ((b & 0x80) == 0) {
		lengthSize = 1;
		return b;
	}

	// 8th bit is set, so lower bits hold the size of the length
	int ls = b & 0x7F;
	if (ls > 8) return -3;
	if (len <= ls) return -1;

	// check for undefined length
	if (ls == 0) {
		lengthSize = 1;
		return -2;
	}

	// check for too big length (signed qint64 overflow)
	b = *(++data);
	if (ls == 8 && ((b & 0x80) != 0)) return -3;

	// calculate length
	lengthSize = ls + 1;
	qint64 retval = b;
	while (--ls > 0) retval = (retval << 8) | *(++data);
	return retval;
}

// returns -1 if not enough data available
// returns -2 if length is undefined (one byte: 0x80)
// returns -3 if too big length was found
inline qint64 decodeTLV(const quint8 * data, int len, quint64 & tagNo, int & tagLen, int & lengthSize)
{
	qint64 & sTag = (qint64 &)tagNo;
	sTag = decodeBERTag(data, len, tagLen);
	if (sTag < 0) return -1;
	return decodeBERLength(data + tagLen, len - tagLen, lengthSize);
}

inline quint8 calcBERlengthSize(qint64 length)
{
	if (length < 0x80) return 1;
	return minSizeOfUInt((quint64)length) + 1;
}

// If length < 0 the undefined length is written.
inline void writeLenBytes(quint8 * pos, qint64 length, quint8 lengthSize)
{
	if (length < 0)      { *pos = 0x80; return; }
	if (lengthSize == 1) { *pos = (quint8)length; return; }
	*(pos++) = (lengthSize - 1) | 0x80;
	quint64 len = (quint64)length;
	while (--lengthSize) *(pos++) = (quint8)(len >> ((lengthSize - 1) * 8));
}

inline void insertBERLength(QByteArray & data, int oldSize)
{
	const qint64 length = data.size() - oldSize;
	const quint8 lengthSize = calcBERlengthSize(length);
	if (lengthSize > 1) data.insert(oldSize, QByteArray(lengthSize - 1, '\0'));
	writeLenBytes((quint8 *)data.constData() + oldSize - 1, length, lengthSize);
}

inline quint8 calcTagLen(quint64 tagNo)
{
	if (tagNo < 0x1F) return 1;
	quint8 tagLen = 2;
	quint64 tn = tagNo >> 7;
	while (tn > 0) {
		++tagLen;
		tn >>= 7;
	}
	return tagLen;
}

inline void writeTagBytes(quint8 * pos, quint64 tagNo, bool constructed, quint8 tagLen)
{
	if (tagLen == 1) {
		*pos = (constructed ? 0xE0 : 0xC0) | tagNo;
	} else {
		*(pos++) = constructed ? 0xFF : 0xDF;
		while (--tagLen > 1) *(pos++) = ((tagNo >> ((tagLen - 1) * 7)) & 0x7F) | 0x80;
		*(pos++) = tagNo & 0x7F;
	}
}

inline void writeNull(QByteArray & data, quint64 tagNo)
{
	if (tagNo > 0) return;
	const quint8 tagLen = calcTagLen(tagNo);
	const int oldSize = data.size();
	data.resize(oldSize + tagLen + 2);
	quint8 * pos = (quint8 *)data.constData() + oldSize;
	writeTagBytes(pos, tagNo, false, tagLen);
	pos[tagLen] = 0x81;
	pos[tagLen + 1] = 0;
}

inline void writeZero(QByteArray & data, quint64 tagNo)
{
	const quint8 tagLen = calcTagLen(tagNo);
	const int oldSize = data.size();
	data.resize(oldSize + tagLen + 1);
	quint8 * pos = (quint8 *)data.constData() + oldSize;
	writeTagBytes(pos, tagNo, false, tagLen);
	pos[tagLen] = 0;
}

class TLWriter
{
public:
	TLWriter(QByteArray & data, quint64 tagNo) :
		data_(data), tagNo_(tagNo), tagLen_(calcTagLen(tagNo))
	{
		const int oldSize = data.size();
		oldSize_ = oldSize + tagLen_ + 1;
		data.resize(oldSize_);
		quint8 * pos = (quint8 *)data.constData() + oldSize;
		writeTagBytes(pos, tagNo, true, tagLen_);
		pos[tagLen_] = '\0';
	}

	~TLWriter()
	{
		if (oldSize_ == data_.size()) {
			if (tagNo_ > 0) {
				data_.resize(oldSize_ - tagLen_ - 1);
			} else {
				data_[oldSize_ - 1] = '\x81';
				data_ += '\0';
			}
		} else {
			insertBERLength(data_, oldSize_);
		}
	}

private:
	QByteArray & data_;
	const quint64 tagNo_;
	const quint8 tagLen_;
	int oldSize_;
};

}}}	// namespace

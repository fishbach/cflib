/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>

#include <cstdint>
#include <cstring>

namespace cflib::serialize::impl {

inline uint8 minSizeOfUInt(uint64 v)
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

inline uint8 minSizeOfInt(int64 v)
{
    if (v >= -128 && v <= 127) return 1;

    if (v > 0) {
        const uint64 uv = (uint64)v;
        const uint8 rv = minSizeOfUInt(uv);
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
    const uint8 rv = h / 8;
    if (v >> (rv * 8 - 1) == -1) return rv;
    return rv + 1;
}

// returns -1 if not enough data available
inline int64 decodeBERTag(const uint8 * data, int len, int & tagLen)
{
    if (len < 1) return -1;
    uint64 tagNo = *data & 0x1F;    // remove Class and P/C
    tagLen = 1;
    if (tagNo == 0x1F) {
        if (len < ++tagLen) return -1;
        uint8 b = *(++data);
        tagNo = b & 0x7F;
        while (b & 0x80) {
            if (len < ++tagLen) return -1;
            b = *(++data);
            tagNo = (tagNo << 7) | (b & 0x7F);
        }
    }
    return (int64)tagNo;
}

// returns -1 if not enough data available
// returns -2 if length is undefined (one byte: 0x80)
// returns -3 if too big length was found
inline int64 decodeBERLength(const uint8 * data, int len, int & lengthSize)
{
    // Is some data available?
    if (len < 1) return -1;

    // If 8th bit is not set, length is in this byte.
    uint8 b = *data;
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

    // check for too big length (signed int64 overflow)
    b = *(++data);
    if (ls == 8 && ((b & 0x80) != 0)) return -3;

    // calculate length
    lengthSize = ls + 1;
    int64 retval = b;
    while (--ls > 0) retval = (retval << 8) | *(++data);
    return retval;
}

// returns -1 if not enough data available
// returns -2 if length is undefined (one byte: 0x80)
// returns -3 if too big length was found
inline int64 decodeTLV(const uint8 * data, int len, uint64 & tagNo, int & tagLen, int & lengthSize)
{
    int64 & sTag = (int64 &)tagNo;
    sTag = decodeBERTag(data, len, tagLen);
    if (sTag < 0) return -1;
    return decodeBERLength(data + tagLen, len - tagLen, lengthSize);
}

inline uint8 calcBERlengthSize(int64 length)
{
    if (length < 0x80) return 1;
    return minSizeOfUInt((uint64)length) + 1;
}

// If length < 0 the undefined length is written.
inline void writeLenBytes(uint8 * pos, int64 length, uint8 lengthSize)
{
    if (length < 0)      { *pos = 0x80; return; }
    if (lengthSize == 1) { *pos = (uint8)length; return; }
    *(pos++) = (lengthSize - 1) | 0x80;
    uint64 len = (uint64)length;
    while (--lengthSize) *(pos++) = (uint8)(len >> ((lengthSize - 1) * 8));
}

inline void insertBERLength(ByteArray & data, int oldSize)
{
    const int64 length = data.size() - oldSize;
    const uint8 lengthSize = calcBERlengthSize(length);
    if (lengthSize > 1) data.insert(oldSize, ByteArray(lengthSize - 1, '\0'));
    writeLenBytes((uint8 *)data.constData() + oldSize - 1, length, lengthSize);
}

inline uint8 calcTagLen(uint64 tagNo)
{
    if (tagNo < 0x1F) return 1;
    uint8 tagLen = 2;
    uint64 tn = tagNo >> 7;
    while (tn > 0) {
        ++tagLen;
        tn >>= 7;
    }
    return tagLen;
}

inline void writeTagBytes(uint8 * pos, uint64 tagNo, bool constructed, uint8 tagLen)
{
    if (tagLen == 1) {
        *pos = (constructed ? 0xE0 : 0xC0) | tagNo;
    } else {
        *(pos++) = constructed ? 0xFF : 0xDF;
        while (--tagLen > 1) *(pos++) = ((tagNo >> ((tagLen - 1) * 7)) & 0x7F) | 0x80;
        *(pos++) = tagNo & 0x7F;
    }
}

inline void writeNull(ByteArray & data, uint64 tagNo)
{
    if (tagNo > 0) return;
    const uint8 tagLen = calcTagLen(tagNo);
    const int oldSize = data.size();
    data.resize(oldSize + tagLen + 2);
    uint8 * pos = (uint8 *)data.constData() + oldSize;
    writeTagBytes(pos, tagNo, false, tagLen);
    pos[tagLen] = 0x81;
    pos[tagLen + 1] = 0;
}

inline void writeZero(ByteArray & data, uint64 tagNo)
{
    const uint8 tagLen = calcTagLen(tagNo);
    const int oldSize = data.size();
    data.resize(oldSize + tagLen + 1);
    uint8 * pos = (uint8 *)data.constData() + oldSize;
    writeTagBytes(pos, tagNo, false, tagLen);
    pos[tagLen] = 0;
}

class TLWriter
{
public:
    TLWriter(ByteArray & data, uint64 tagNo) :
        data_(data), tagNo_(tagNo), tagLen_(calcTagLen(tagNo))
    {
        const int oldSize = data.size();
        oldSize_ = oldSize + tagLen_ + 1;
        data.resize(oldSize_);
        uint8 * pos = (uint8 *)data.constData() + oldSize;
        writeTagBytes(pos, tagNo, true, tagLen_);
        pos[tagLen_] = '\0';
    }

    ~TLWriter()
    {
        if (oldSize_ == (int)data_.size()) {
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
    ByteArray & data_;
    const uint64 tagNo_;
    const uint8 tagLen_;
    int oldSize_;
};

} // namespace

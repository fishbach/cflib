/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>

#include <bit>
#include <cstring>

namespace
{

// Byte-swap helpers (replacing cfToBigEndian/cfFromBigEndian)
template<typename T>
inline void cfToBigEndian(T val, uint8 * dest) {
    // Convert to big-endian and write to dest
    if constexpr (std::endian::native == std::endian::little) {
        uint8 * src = reinterpret_cast<uint8 *>(&val);
        for (int i = 0; i < (int)sizeof(T); ++i)
            dest[i] = src[sizeof(T) - 1 - i];
    } else {
        std::memcpy(dest, &val, sizeof(T));
    }
}

template<typename T>
inline T cfFromBigEndian(const uint8 * src) {
    T val;
    if constexpr (std::endian::native == std::endian::little) {
        uint8 * dest = reinterpret_cast<uint8 *>(&val);
        for (int i = 0; i < (int)sizeof(T); ++i)
            dest[i] = src[sizeof(T) - 1 - i];
    } else {
        std::memcpy(&val, src, sizeof(T));
    }
    return val;
}

}

namespace cflib::net::impl {

class KafkaString : public ByteArray
{
public:
    inline KafkaString() {}
    inline KafkaString(const char * data, int size = -1) : ByteArray(data, size) {}
    inline KafkaString(int size, char c) : ByteArray(size, c) {}
    inline KafkaString(const ByteArray & other) : ByteArray(other) {}
};

class KafkaRawWriter
{
public:
    KafkaRawWriter(uint32 expectedSize = 0)
    {
        data_.reserve(4 + expectedSize);
        data_.resize(4);
    }

    inline void reset()
    {
        data_.resize(4);
    }

    inline ByteArray getData()
    {
        cfToBigEndian<int32>(data_.size() - 4, (uint8 *)data_.constData());
        return data_;
    }

    inline void expectMoreBytes(uint32 count)
    {
        data_.reserve(data_.size() + count);
    }

    inline uint8 * getCurrentRawData()
    {
        return (uint8 *)data_.data();
    }

    inline int getCurrentSize()
    {
        return data_.size();
    }

    inline ByteArray getRawContent()
    {
        return data_.mid(4);
    }

private:
    ByteArray data_;

    friend KafkaRawWriter & operator<<(KafkaRawWriter & out, int8               val);
    friend KafkaRawWriter & operator<<(KafkaRawWriter & out, int16              val);
    friend KafkaRawWriter & operator<<(KafkaRawWriter & out, int32              val);
    friend KafkaRawWriter & operator<<(KafkaRawWriter & out, int64              val);
    friend KafkaRawWriter & operator<<(KafkaRawWriter & out, const ByteArray  & val);
    friend KafkaRawWriter & operator<<(KafkaRawWriter & out, const KafkaString & val);
};

#define DEFINE_KAFKA_WRITE_INT_OPERATOR(type) \
    inline KafkaRawWriter & operator<<(KafkaRawWriter & out, type val) \
    { \
        const int oldSize = out.data_.size(); \
        out.data_.resize(oldSize + sizeof(type)); \
        cfToBigEndian<type>(val, (uint8 *)(out.data_.constData() + oldSize)); \
        return out; \
    } \

DEFINE_KAFKA_WRITE_INT_OPERATOR(int8)
DEFINE_KAFKA_WRITE_INT_OPERATOR(int16)
DEFINE_KAFKA_WRITE_INT_OPERATOR(int32)
DEFINE_KAFKA_WRITE_INT_OPERATOR(int64)

inline KafkaRawWriter & operator<<(KafkaRawWriter & out, const ByteArray & val)
{
    if (val.isNull()) {
        out << (int32)-1;
    } else {
        out << (int32)val.size();
        out.data_.append(val);
    }
    return out;
}

inline KafkaRawWriter & operator<<(KafkaRawWriter & out, const KafkaString & val)
{
    if (val.isNull()) {
        out << (int16)-1;
    } else {
        out << (int16)val.size();
        out.data_.append(val);
    }
    return out;
}


class KafkaRawReader
{
public:
    KafkaRawReader(const ByteArray & data) :
        readPtr_(data.constData()),
        bytesLeft_(data.size())
    {
    }

    inline uint32 bytesLeft() { return bytesLeft_; }

private:
    const char * readPtr_;
    uint32 bytesLeft_;

    friend KafkaRawReader & operator>>(KafkaRawReader & in, int8       & val);
    friend KafkaRawReader & operator>>(KafkaRawReader & in, int16      & val);
    friend KafkaRawReader & operator>>(KafkaRawReader & in, int32      & val);
    friend KafkaRawReader & operator>>(KafkaRawReader & in, int64      & val);
    friend KafkaRawReader & operator>>(KafkaRawReader & in, ByteArray  & val);
    friend KafkaRawReader & operator>>(KafkaRawReader & in, KafkaString & val);
};

#define DEFINE_KAFKA_READ_INT_OPERATOR(type) \
    inline KafkaRawReader & operator>>(KafkaRawReader & in, type & val) \
    { \
        if (sizeof(type) > in.bytesLeft_) { \
            val = 0; \
            in.bytesLeft_ = 0; \
        } else { \
            val = cfFromBigEndian<type>((const uint8 *)in.readPtr_); \
            in.readPtr_   += sizeof(type); \
            in.bytesLeft_ -= sizeof(type); \
        } \
        return in; \
    } \

DEFINE_KAFKA_READ_INT_OPERATOR(int8)
DEFINE_KAFKA_READ_INT_OPERATOR(int16)
DEFINE_KAFKA_READ_INT_OPERATOR(int32)
DEFINE_KAFKA_READ_INT_OPERATOR(int64)

inline KafkaRawReader & operator>>(KafkaRawReader & in, ByteArray & val)
{
    int32 size;
    in >> size;
    if (size < 0) {
        val = ByteArray();
    } else {
        if ((uint32)size > in.bytesLeft_) size = in.bytesLeft_;
        val = ByteArray(in.readPtr_, size);
        in.readPtr_   += size;
        in.bytesLeft_ -= size;
    }
    return in;
}

inline KafkaRawReader & operator>>(KafkaRawReader & in, KafkaString & val)
{
    int16 size;
    in >> size;
    if (size < 0) {
        val = KafkaString();
    } else {
        if ((uint32)size > in.bytesLeft_) size = in.bytesLeft_;
        val = KafkaString(in.readPtr_, size);
        in.readPtr_   += size;
        in.bytesLeft_ -= size;
    }
    return in;
}

} // namespace

/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>
#include <cflib/serialize/impl/registerclass.h>
#include <cflib/serialize/impl/serializebaseber.h>

namespace cflib::serialize::impl {

// ============================================================================
// integer
// ============================================================================

template<typename T>
inline void serializeBERInt(T v, uint64 tagNo, ByteArray & data, bool isMaxUInt = false)
{
    if (v == 0) { writeNull(data, tagNo); return; }

    uint8 len = isMaxUInt ? 9 : minSizeOfInt(v);

    const uint8 tagLen = calcTagLen(tagNo);
    const int size = data.size();
    data.resize(size + tagLen + 1 + len);
    uint8 * p = (uint8 *)data.data() + size;

    // write tag
    writeTagBytes(p, tagNo, false, tagLen);
    p += tagLen;

    // write len
    *p = len;

    // write value
    p += len;
    *p = (uint8)v;
    while (--len > 0) {
        if (sizeof(v) == 1) {
            *(--p) = 0;
        } else {
            v >>= 8;
            *(--p) = (uint8)v;
        }
    }
}

template<>
inline void serializeBERInt(bool v, uint64 tagNo, ByteArray & data, bool)
{
    if (!v) { writeNull(data, tagNo); return; }

    const uint8 tagLen = calcTagLen(tagNo);
    const int size = data.size();
    data.resize(size + tagLen + 2);
    uint8 * p = (uint8 *)data.data() + size;

    // write tag
    writeTagBytes(p, tagNo, false, tagLen);

    // write len
    p[tagLen] = 1;

    // write value
    p[tagLen + 1] = 1;
}

inline void serializeBERInt(uint64 v, uint64 tagNo, ByteArray & data)
{
    serializeBERInt<uint64>(v, tagNo, data, (v >> 63));
}

template<typename T>
inline void deserializeBERInt(T & v, const uint8 * data, int len)
{
    if (len < 1) {
        v = 0;
        return;
    }
    const uint8 * b = (const uint8 *)data;
    v = *b;
    if (v & 0x80) v = ((T)-1 & ~0xFF) | v;
    while (--len > 0) v = (v << 8) | *(++b);
}

template<>
inline void deserializeBERInt(bool & v, const uint8 * data, int len)
{
    v = len > 0 && (uint8)*data == 1;
}

// ----------------------------------------------------------------------------

#define DO_SERIALIZE_BER(typ) \
    inline void serializeBER(typ val, uint64 tag, ByteArray & data, BERSerializerBase &) { \
        serializeBERInt(val, tag, data); } \
    inline void deserializeBER(typ & val, const uint8 * data, int len, BERDeserializerBase &) { \
        deserializeBERInt(val, data, len); } \

DO_SERIALIZE_BER(bool   )
DO_SERIALIZE_BER(int8  )
DO_SERIALIZE_BER(uint8 )
DO_SERIALIZE_BER(int16 )
DO_SERIALIZE_BER(uint16)
DO_SERIALIZE_BER(int32 )
DO_SERIALIZE_BER(uint32)
DO_SERIALIZE_BER(int64 )
DO_SERIALIZE_BER(uint64)


// ============================================================================
// float
// ============================================================================

inline void serializeBER(float d, uint64 tagNo, ByteArray & data, BERSerializerBase &)
{
    if (d == 0) { writeNull(data, tagNo); return; }
    const uint8 tagLen = calcTagLen(tagNo);
    const int oldSize = data.size();
    data.resize(oldSize + tagLen + 5);
    uint8 * pos = (uint8 *)data.data() + oldSize;
    writeTagBytes(pos, tagNo, false, tagLen);
    pos += tagLen;
    *(pos++) = 4;
    memcpy(pos, (const char *)&d, 4);
}

inline void deserializeBER(float & d, const uint8 * data, int len, BERDeserializerBase &)
{
    if (len != 4) d = 0;
    else memcpy(&d, data, sizeof(float));
}

inline void serializeBER(double d, uint64 tagNo, ByteArray & data, BERSerializerBase &)
{
    if (d == 0) { writeNull(data, tagNo); return; }
    const uint8 tagLen = calcTagLen(tagNo);
    const int oldSize = data.size();
    data.resize(oldSize + tagLen + 9);
    uint8 * pos = (uint8 *)data.data() + oldSize;
    writeTagBytes(pos, tagNo, false, tagLen);
    pos += tagLen;
    *(pos++) = 8;
    memcpy(pos, (const char *)&d, 8);
}

inline void deserializeBER(double & d, const uint8 * data, int len, BERDeserializerBase &)
{
    if (len != 8) d = 0;
    else memcpy(&d, data, sizeof(double));
}

inline void serializeBER(long double d, uint64 tagNo, ByteArray & data, BERSerializerBase &)
{
    if (d == 0) { writeNull(data, tagNo); return; }
    const uint8 tagLen = calcTagLen(tagNo);
    const int oldSize = data.size();
    data.resize(oldSize + tagLen + 17);
    uint8 * pos = (uint8 *)data.data() + oldSize;
    writeTagBytes(pos, tagNo, false, tagLen);
    pos += tagLen;
    *(pos++) = 16;
    memcpy(pos, (const char *)&d, 16);
}

inline void deserializeBER(long double & d, const uint8 * data, int len, BERDeserializerBase &)
{
    if (len != 16) d = 0;
    else memcpy(&d, data, sizeof(long double));
}


// ============================================================================
// strings
// ============================================================================

// ----------------------------------------------------------------------------
// ByteArray
// ----------------------------------------------------------------------------

inline void serializeBER(const ByteArray & ba, uint64 tagNo, ByteArray & data, BERSerializerBase &)
{
    if (ba.isNull())  { writeNull(data, tagNo); return; }
    if (ba.isEmpty()) { writeZero(data, tagNo); return; }

    const uint8 tagLen = calcTagLen(tagNo);
    const uint8 lengthSize = calcBERlengthSize(ba.size());
    const int oldSize = data.size();
    data.resize(oldSize + tagLen + lengthSize + ba.size());
    uint8 * pos = (uint8 *)data.data() + oldSize;
    writeTagBytes(pos, tagNo, false, tagLen);
    writeLenBytes(pos + tagLen, ba.size(), lengthSize);
    memcpy(pos + tagLen + lengthSize, ba.constData(), ba.size());
}

inline void deserializeBER(ByteArray & ba, const uint8 * data, int len, BERDeserializerBase &)
{
    ba = ByteArray((const char *)data, (size_t)len);
}

// ----------------------------------------------------------------------------
// String
// ----------------------------------------------------------------------------

inline void serializeBER(const String & str, uint64 tagNo, ByteArray & data, BERSerializerBase & ser)
{
    if (str.isNull())  { writeNull(data, tagNo); return; }
    if (str.isEmpty()) { writeZero(data, tagNo); return; }
    ByteArray utf8 = str.toUtf8();
    serializeBER(utf8, tagNo, data, ser);
}

inline void deserializeBER(String & str, const uint8 * data, int len, BERDeserializerBase &)
{
    str = String::fromUtf8((const char *)data, (size_t)len);
}

// ----------------------------------------------------------------------------
// const char * (use ByteArray for deserialization)
// ----------------------------------------------------------------------------

inline void serializeBER(const char * str, uint64 tagNo, ByteArray & data, BERSerializerBase &)
{
    if (str == 0) { writeNull(data, tagNo); return; }

    const int64 len = strlen(str);
    if (len == 0) { writeZero(data, tagNo); return; }

    const uint8 tagLen = calcTagLen(tagNo);
    const uint8 lengthSize = calcBERlengthSize(len);
    const int oldSize = data.size();
    data.resize(oldSize + tagLen + lengthSize + len);
    uint8 * pos = (uint8 *)data.data() + oldSize;
    writeTagBytes(pos, tagNo, false, tagLen);
    writeLenBytes(pos + tagLen, len, lengthSize);
    memcpy(pos + tagLen + lengthSize, str, len);
}


// ============================================================================
// CF classes
// ============================================================================

// ----------------------------------------------------------------------------
// DateTime
// ----------------------------------------------------------------------------

inline void serializeBER(const DateTime & dt, uint64 tagNo, ByteArray & data, BERSerializerBase &)
{
    if (!dt.isValid()) { writeNull(data, tagNo); return; }
    serializeBERInt(dt.toMSecsSinceEpoch(), tagNo, data);
}

inline void deserializeBER(DateTime & dt, const uint8 * data, int len, BERDeserializerBase &)
{
    int64 msec;
    deserializeBERInt(msec, data, len);
    dt = DateTime::fromMSecsSinceEpoch(msec);
}

// ----------------------------------------------------------------------------
// Flags
// ----------------------------------------------------------------------------

template<typename T>
inline void serializeBER(const Flags<T> & fl, uint64 tagNo, ByteArray & data, BERSerializerBase &)
{
    serializeBERInt((int32)fl.toInt(), tagNo, data);
}

template<typename T>
inline void deserializeBER(Flags<T> & fl, const uint8 * data, int len, BERDeserializerBase &)
{
    int32 flags;
    deserializeBERInt(flags, data, len);
    fl = Flags<T>((T)flags);
}


// ============================================================================
// container types
// ============================================================================

// ----------------------------------------------------------------------------
// Pair
// ----------------------------------------------------------------------------

template<typename T1, typename T2>
inline void serializeBER(const Pair<T1, T2> & cl, uint64 tagNo, ByteArray & data, BERSerializerBase &)
{
    TLWriter tlw(data, tagNo);
    BERSerializerBase ser(data);
    ser << cl.first << cl.second;
}

template<typename T1, typename T2>
inline void deserializeBER(Pair<T1, T2> & cl, const uint8 * data, int len, BERDeserializerBase &)
{
    BERDeserializerBase ser(data, len);
    ser >> cl.first >> cl.second;
}

// ----------------------------------------------------------------------------
// List
// ----------------------------------------------------------------------------

template<typename T>
inline void serializeBER(const List<T> & cl, uint64 tagNo, ByteArray & data, BERSerializerBase &)
{
    TLWriter tlw(data, tagNo);
    BERSerializerBase ser(data, true);
    for (auto it = cl.begin() ; it != cl.end() ; ++it) ser << (T)*it;
}

template<typename T>
inline void deserializeBER(List<T> & cl, const uint8 * data, int len, BERDeserializerBase &)
{
    BERDeserializerBase ser(data, len, true);
    cl.clear();
    while (ser.isAnyAvailable()) {
        T el; ser >> el;
        cl.push_back(el);
    }
}

// ----------------------------------------------------------------------------
// Set
// ----------------------------------------------------------------------------

template<typename T>
inline void serializeBER(const Set<T> & cl, uint64 tagNo, ByteArray & data, BERSerializerBase &)
{
    TLWriter tlw(data, tagNo);
    BERSerializerBase ser(data, true);
    if (cl.empty()) return;
    for (auto it = cl.begin() ; it != cl.end() ; ++it) ser << *it;
}

template<typename T>
inline void deserializeBER(Set<T> & cl, const uint8 * data, int len, BERDeserializerBase &)
{
    BERDeserializerBase ser(data, len, true);
    cl.clear();
    while (ser.isAnyAvailable()) {
        T el; ser >> el;
        cl.insert(el);
    }
}

// ----------------------------------------------------------------------------
// Hash
// ----------------------------------------------------------------------------

template<typename Key, typename T>
inline void serializeBER(const Hash<Key, T> & cl, uint64 tagNo, ByteArray & data, BERSerializerBase &)
{
    TLWriter tlw(data, tagNo);
    BERSerializerBase ser(data, true);
    for (auto it = cl.begin() ; it != cl.end() ; ++it) {
        ser << it->first << it->second;
    }
}

template<typename Key, typename T>
inline void deserializeBER(Hash<Key, T> & cl, const uint8 * data, int len, BERDeserializerBase &)
{
    BERDeserializerBase ser(data, len, true);
    cl.clear();
    while (ser.isAnyAvailable()) {
        Key key;
        T value;
        ser >> key >> value;
        cl[key] = value;
    }
}

// ----------------------------------------------------------------------------
// MultiHash
// ----------------------------------------------------------------------------

template<typename Key, typename T>
inline void serializeBER(const MultiHash<Key, T> & cl, uint64 tagNo, ByteArray & data, BERSerializerBase &)
{
    TLWriter tlw(data, tagNo);
    BERSerializerBase ser(data, true);
    for (auto it = cl.begin() ; it != cl.end() ; ++it) {
        ser << it->first << it->second;
    }
}

template<typename Key, typename T>
inline void deserializeBER(MultiHash<Key, T> & cl, const uint8 * data, int len, BERDeserializerBase &)
{
    BERDeserializerBase ser(data, len, true);
    cl.clear();
    while (ser.isAnyAvailable()) {
        Key key;
        T value;
        ser >> key >> value;
        cl.insert({key, value});
    }
}

// ----------------------------------------------------------------------------
// Map
// ----------------------------------------------------------------------------

template<typename Key, typename T>
inline void serializeBER(const Map<Key, T> & cl, uint64 tagNo, ByteArray & data, BERSerializerBase &)
{
    TLWriter tlw(data, tagNo);
    BERSerializerBase ser(data, true);
    for (auto it = cl.begin() ; it != cl.end() ; ++it) {
        ser << it->first << it->second;
    }
}

template<typename Key, typename T>
inline void deserializeBER(Map<Key, T> & cl, const uint8 * data, int len, BERDeserializerBase &)
{
    BERDeserializerBase ser(data, len, true);
    cl.clear();
    while (ser.isAnyAvailable()) {
        Key key;
        T value;
        ser >> key >> value;
        cl[key] = value;
    }
}

// ----------------------------------------------------------------------------
// MultiMap
// ----------------------------------------------------------------------------

template<typename Key, typename T>
inline void serializeBER(const MultiMap<Key, T> & cl, uint64 tagNo, ByteArray & data, BERSerializerBase &)
{
    TLWriter tlw(data, tagNo);
    BERSerializerBase ser(data, true);
    for (auto it = cl.begin() ; it != cl.end() ; ++it) {
        ser << it->first << it->second;
    }
}

template<typename Key, typename T>
inline void deserializeBER(MultiMap<Key, T> & cl, const uint8 * data, int len, BERDeserializerBase &)
{
    BERDeserializerBase ser(data, len, true);
    cl.clear();
    while (ser.isAnyAvailable()) {
        Key key;
        T value;
        ser >> key >> value;
        cl.insert({key, value});
    }
}


// ============================================================================
// custom classes
// ============================================================================

template<typename T>
inline void serializeBER(const T & cl, uint64 tagNo, ByteArray & data, BERSerializerBase &)
{
    TLWriter tlw(data, tagNo);
    BERSerializerBase ser(data);
    cl.serialize(ser);
}

template<typename T>
inline void deserializeBER(T & cl, const uint8 * data, int len, BERDeserializerBase &)
{
    BERDeserializerBase ser(data, len);
    cl.deserialize(ser);
}


// ============================================================================
// dynamic classes
// ============================================================================

template<typename T>
inline void serializeBER(const SharedPtr<T> & cl, uint64 tagNo, ByteArray & data, BERSerializerBase &)
{
    if (!cl) return;

    TLWriter tlw(data, tagNo);
    BERSerializerBase ser(data);
    RegisterClassBase::serialize(cl, ser);
}

template<typename T>
inline void deserializeBER(SharedPtr<T> & cl, const uint8 * data, int len, BERDeserializerBase &)
{
    RegisterClassBase::deserialize(cl, data, len);
}


} // namespace

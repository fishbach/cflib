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

namespace cflib { namespace serialize { namespace impl {

// ============================================================================
// integer
// ============================================================================

template<typename T>
inline void serializeBERInt(T v, cfuint64 tagNo, CFByteArray & data, bool isMaxUInt = false)
{
    if (v == 0) { writeNull(data, tagNo); return; }

    cfuint8 len = isMaxUInt ? 9 : minSizeOfInt(v);

    const cfuint8 tagLen = calcTagLen(tagNo);
    const int size = data.size();
    data.resize(size + tagLen + 1 + len);
    cfuint8 * p = (cfuint8 *)data.data() + size;

    // write tag
    writeTagBytes(p, tagNo, false, tagLen);
    p += tagLen;

    // write len
    *p = len;

    // write value
    p += len;
    *p = (cfuint8)v;
    while (--len > 0) {
        if (sizeof(v) == 1) {
            *(--p) = 0;
        } else {
            v >>= 8;
            *(--p) = (cfuint8)v;
        }
    }
}

template<>
inline void serializeBERInt(bool v, cfuint64 tagNo, CFByteArray & data, bool)
{
    if (!v) { writeNull(data, tagNo); return; }

    const cfuint8 tagLen = calcTagLen(tagNo);
    const int size = data.size();
    data.resize(size + tagLen + 2);
    cfuint8 * p = (cfuint8 *)data.data() + size;

    // write tag
    writeTagBytes(p, tagNo, false, tagLen);

    // write len
    p[tagLen] = 1;

    // write value
    p[tagLen + 1] = 1;
}

inline void serializeBERInt(cfuint64 v, cfuint64 tagNo, CFByteArray & data)
{
    serializeBERInt<cfuint64>(v, tagNo, data, (v >> 63));
}

template<typename T>
inline void deserializeBERInt(T & v, const cfuint8 * data, int len)
{
    if (len < 1) {
        v = 0;
        return;
    }
    const cfuint8 * b = (const cfuint8 *)data;
    v = *b;
    if (v & 0x80) v = ((T)-1 & ~0xFF) | v;
    while (--len > 0) v = (v << 8) | *(++b);
}

template<>
inline void deserializeBERInt(bool & v, const cfuint8 * data, int len)
{
    v = len > 0 && (cfuint8)*data == 1;
}

// ----------------------------------------------------------------------------

#define DO_SERIALIZE_BER(typ) \
    inline void serializeBER(typ val, cfuint64 tag, CFByteArray & data, BERSerializerBase &) { \
        serializeBERInt(val, tag, data); } \
    inline void deserializeBER(typ & val, const cfuint8 * data, int len, BERDeserializerBase &) { \
        deserializeBERInt(val, data, len); } \

DO_SERIALIZE_BER(bool   )
DO_SERIALIZE_BER(cfint8  )
DO_SERIALIZE_BER(cfuint8 )
DO_SERIALIZE_BER(cfint16 )
DO_SERIALIZE_BER(cfuint16)
DO_SERIALIZE_BER(cfint32 )
DO_SERIALIZE_BER(cfuint32)
DO_SERIALIZE_BER(cfint64 )
DO_SERIALIZE_BER(cfuint64)


// ============================================================================
// float
// ============================================================================

inline void serializeBER(float d, cfuint64 tagNo, CFByteArray & data, BERSerializerBase &)
{
    if (d == 0) { writeNull(data, tagNo); return; }
    const cfuint8 tagLen = calcTagLen(tagNo);
    const int oldSize = data.size();
    data.resize(oldSize + tagLen + 5);
    cfuint8 * pos = (cfuint8 *)data.data() + oldSize;
    writeTagBytes(pos, tagNo, false, tagLen);
    pos += tagLen;
    *(pos++) = 4;
    memcpy(pos, (const char *)&d, 4);
}

inline void deserializeBER(float & d, const cfuint8 * data, int len, BERDeserializerBase &)
{
    d = len != 4 ? 0 : *((const float *)data);
}

inline void serializeBER(double d, cfuint64 tagNo, CFByteArray & data, BERSerializerBase &)
{
    if (d == 0) { writeNull(data, tagNo); return; }
    const cfuint8 tagLen = calcTagLen(tagNo);
    const int oldSize = data.size();
    data.resize(oldSize + tagLen + 9);
    cfuint8 * pos = (cfuint8 *)data.data() + oldSize;
    writeTagBytes(pos, tagNo, false, tagLen);
    pos += tagLen;
    *(pos++) = 8;
    memcpy(pos, (const char *)&d, 8);
}

inline void deserializeBER(double & d, const cfuint8 * data, int len, BERDeserializerBase &)
{
    d = len != 8 ? 0 : *((const double *)data);
}

inline void serializeBER(long double d, cfuint64 tagNo, CFByteArray & data, BERSerializerBase &)
{
    if (d == 0) { writeNull(data, tagNo); return; }
    const cfuint8 tagLen = calcTagLen(tagNo);
    const int oldSize = data.size();
    data.resize(oldSize + tagLen + 17);
    cfuint8 * pos = (cfuint8 *)data.data() + oldSize;
    writeTagBytes(pos, tagNo, false, tagLen);
    pos += tagLen;
    *(pos++) = 16;
    memcpy(pos, (const char *)&d, 16);
}

inline void deserializeBER(long double & d, const cfuint8 * data, int len, BERDeserializerBase &)
{
    d = len != 16 ? 0 : *((const long double *)data);
}


// ============================================================================
// strings
// ============================================================================

// ----------------------------------------------------------------------------
// CFByteArray
// ----------------------------------------------------------------------------

inline void serializeBER(const CFByteArray & ba, cfuint64 tagNo, CFByteArray & data, BERSerializerBase &)
{
    if (ba.isNull())  { writeNull(data, tagNo); return; }
    if (ba.isEmpty()) { writeZero(data, tagNo); return; }

    const cfuint8 tagLen = calcTagLen(tagNo);
    const cfuint8 lengthSize = calcBERlengthSize(ba.size());
    const int oldSize = data.size();
    data.resize(oldSize + tagLen + lengthSize + ba.size());
    cfuint8 * pos = (cfuint8 *)data.data() + oldSize;
    writeTagBytes(pos, tagNo, false, tagLen);
    writeLenBytes(pos + tagLen, ba.size(), lengthSize);
    memcpy(pos + tagLen + lengthSize, ba.constData(), ba.size());
}

inline void deserializeBER(CFByteArray & ba, const cfuint8 * data, int len, BERDeserializerBase &)
{
    ba = CFByteArray((const char *)data, (cfsize_t)len);
}

// ----------------------------------------------------------------------------
// CFString
// ----------------------------------------------------------------------------

inline void serializeBER(const CFString & str, cfuint64 tagNo, CFByteArray & data, BERSerializerBase & ser)
{
    if (str.isNull())  { writeNull(data, tagNo); return; }
    if (str.isEmpty()) { writeZero(data, tagNo); return; }
    CFByteArray utf8 = str.toUtf8();
    serializeBER(utf8, tagNo, data, ser);
}

inline void deserializeBER(CFString & str, const cfuint8 * data, int len, BERDeserializerBase &)
{
    str = CFString::fromUtf8((const char *)data, (cfsize_t)len);
}

// ----------------------------------------------------------------------------
// const char * (use CFByteArray for deserialization)
// ----------------------------------------------------------------------------

inline void serializeBER(const char * str, cfuint64 tagNo, CFByteArray & data, BERSerializerBase &)
{
    if (str == 0) { writeNull(data, tagNo); return; }

    const cfint64 len = strlen(str);
    if (len == 0) { writeZero(data, tagNo); return; }

    const cfuint8 tagLen = calcTagLen(tagNo);
    const cfuint8 lengthSize = calcBERlengthSize(len);
    const int oldSize = data.size();
    data.resize(oldSize + tagLen + lengthSize + len);
    cfuint8 * pos = (cfuint8 *)data.data() + oldSize;
    writeTagBytes(pos, tagNo, false, tagLen);
    writeLenBytes(pos + tagLen, len, lengthSize);
    memcpy(pos + tagLen + lengthSize, str, len);
}

// ----------------------------------------------------------------------------
// CFChar
// ----------------------------------------------------------------------------

inline void serializeBER(const CFChar & c, cfuint64 tagNo, CFByteArray & data, BERSerializerBase &)
{
    if (c.isNull())  { writeNull(data, tagNo); return; }
    serializeBERInt((cfuint16)(c.unicode()), tagNo, data);
}

inline void deserializeBER(CFChar & c, const cfuint8 * data, int len, BERDeserializerBase &)
{
    cfuint16 unicode;
    deserializeBERInt(unicode, data, len);
    c = CFChar((char32_t)unicode);
}


// ============================================================================
// CF classes
// ============================================================================

// ----------------------------------------------------------------------------
// CFDateTime
// ----------------------------------------------------------------------------

inline void serializeBER(const CFDateTime & dt, cfuint64 tagNo, CFByteArray & data, BERSerializerBase &)
{
    if (!dt.isValid()) { writeNull(data, tagNo); return; }
    serializeBERInt(dt.toMSecsSinceEpoch(), tagNo, data);
}

inline void deserializeBER(CFDateTime & dt, const cfuint8 * data, int len, BERDeserializerBase &)
{
    cfint64 msec;
    deserializeBERInt(msec, data, len);
    dt = CFDateTime::fromMSecsSinceEpoch(msec);
}

// ----------------------------------------------------------------------------
// CFFlags
// ----------------------------------------------------------------------------

template<typename T>
inline void serializeBER(const CFFlags<T> & fl, cfuint64 tagNo, CFByteArray & data, BERSerializerBase &)
{
    serializeBERInt((cfint32)fl.toInt(), tagNo, data);
}

template<typename T>
inline void deserializeBER(CFFlags<T> & fl, const cfuint8 * data, int len, BERDeserializerBase &)
{
    cfint32 flags;
    deserializeBERInt(flags, data, len);
    fl = CFFlags<T>((T)flags);
}


// ============================================================================
// CF container types
// ============================================================================

// ----------------------------------------------------------------------------
// CFPair
// ----------------------------------------------------------------------------

template<typename T1, typename T2>
inline void serializeBER(const CFPair<T1, T2> & cl, cfuint64 tagNo, CFByteArray & data, BERSerializerBase &)
{
    TLWriter tlw(data, tagNo);
    BERSerializerBase ser(data);
    ser << cl.first << cl.second;
}

template<typename T1, typename T2>
inline void deserializeBER(CFPair<T1, T2> & cl, const cfuint8 * data, int len, BERDeserializerBase &)
{
    BERDeserializerBase ser(data, len);
    ser >> cl.first >> cl.second;
}

// ----------------------------------------------------------------------------
// CFList
// ----------------------------------------------------------------------------

template<typename T>
inline void serializeBER(const CFList<T> & cl, cfuint64 tagNo, CFByteArray & data, BERSerializerBase &)
{
    TLWriter tlw(data, tagNo);
    BERSerializerBase ser(data, true);
    for (auto it = cl.begin() ; it != cl.end() ; ++it) ser << (T)*it;
}

template<typename T>
inline void deserializeBER(CFList<T> & cl, const cfuint8 * data, int len, BERDeserializerBase &)
{
    BERDeserializerBase ser(data, len, true);
    cl.clear();
    while (ser.isAnyAvailable()) {
        T el; ser >> el;
        cl.push_back(el);
    }
}

// ----------------------------------------------------------------------------
// CFStringList is CFList<CFString> — handled by the CFList template above.

// ----------------------------------------------------------------------------
// CFVector (same as CFList)
// ----------------------------------------------------------------------------

// CFVector is the same type as CFList (both std::vector), no separate overload needed.

// ----------------------------------------------------------------------------
// CFSet
// ----------------------------------------------------------------------------

template<typename T>
inline void serializeBER(const CFSet<T> & cl, cfuint64 tagNo, CFByteArray & data, BERSerializerBase &)
{
    TLWriter tlw(data, tagNo);
    BERSerializerBase ser(data, true);
    if (cl.empty()) return;
    for (auto it = cl.begin() ; it != cl.end() ; ++it) ser << *it;
}

template<typename T>
inline void deserializeBER(CFSet<T> & cl, const cfuint8 * data, int len, BERDeserializerBase &)
{
    BERDeserializerBase ser(data, len, true);
    cl.clear();
    while (ser.isAnyAvailable()) {
        T el; ser >> el;
        cl.insert(el);
    }
}

// ----------------------------------------------------------------------------
// CFHash
// ----------------------------------------------------------------------------

template<typename Key, typename T>
inline void serializeBER(const CFHash<Key, T> & cl, cfuint64 tagNo, CFByteArray & data, BERSerializerBase &)
{
    TLWriter tlw(data, tagNo);
    BERSerializerBase ser(data, true);
    for (auto it = cl.begin() ; it != cl.end() ; ++it) {
        ser << it->first << it->second;
    }
}

template<typename Key, typename T>
inline void deserializeBER(CFHash<Key, T> & cl, const cfuint8 * data, int len, BERDeserializerBase &)
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
// CFMultiHash
// ----------------------------------------------------------------------------

template<typename Key, typename T>
inline void serializeBER(const CFMultiHash<Key, T> & cl, cfuint64 tagNo, CFByteArray & data, BERSerializerBase &)
{
    TLWriter tlw(data, tagNo);
    BERSerializerBase ser(data, true);
    for (auto it = cl.begin() ; it != cl.end() ; ++it) {
        ser << it->first << it->second;
    }
}

template<typename Key, typename T>
inline void deserializeBER(CFMultiHash<Key, T> & cl, const cfuint8 * data, int len, BERDeserializerBase &)
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
// CFMap
// ----------------------------------------------------------------------------

template<typename Key, typename T>
inline void serializeBER(const CFMap<Key, T> & cl, cfuint64 tagNo, CFByteArray & data, BERSerializerBase &)
{
    TLWriter tlw(data, tagNo);
    BERSerializerBase ser(data, true);
    for (auto it = cl.begin() ; it != cl.end() ; ++it) {
        ser << it->first << it->second;
    }
}

template<typename Key, typename T>
inline void deserializeBER(CFMap<Key, T> & cl, const cfuint8 * data, int len, BERDeserializerBase &)
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
// CFMultiMap
// ----------------------------------------------------------------------------

template<typename Key, typename T>
inline void serializeBER(const CFMultiMap<Key, T> & cl, cfuint64 tagNo, CFByteArray & data, BERSerializerBase &)
{
    TLWriter tlw(data, tagNo);
    BERSerializerBase ser(data, true);
    for (auto it = cl.begin() ; it != cl.end() ; ++it) {
        ser << it->first << it->second;
    }
}

template<typename Key, typename T>
inline void deserializeBER(CFMultiMap<Key, T> & cl, const cfuint8 * data, int len, BERDeserializerBase &)
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
inline void serializeBER(const T & cl, cfuint64 tagNo, CFByteArray & data, BERSerializerBase &)
{
    TLWriter tlw(data, tagNo);
    BERSerializerBase ser(data);
    cl.serialize(ser);
}

template<typename T>
inline void deserializeBER(T & cl, const cfuint8 * data, int len, BERDeserializerBase &)
{
    BERDeserializerBase ser(data, len);
    cl.deserialize(ser);
}


// ============================================================================
// dynamic classes
// ============================================================================

template<typename T>
inline void serializeBER(const CFSharedPtr<T> & cl, cfuint64 tagNo, CFByteArray & data, BERSerializerBase &)
{
    if (!cl) return;

    TLWriter tlw(data, tagNo);
    BERSerializerBase ser(data);
    RegisterClassBase::serialize(cl, ser);
}

template<typename T>
inline void deserializeBER(CFSharedPtr<T> & cl, const cfuint8 * data, int len, BERDeserializerBase &)
{
    RegisterClassBase::deserialize(cl, data, len);
}


}}}    // namespace

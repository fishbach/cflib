/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/serialize/impl/readandcall.h>
#include <cflib/serialize/impl/sometobytearray.h>

namespace cflib::serialize {

// returns the value length of the BER TLV
// returns -1 if not enough data available
// returns -2 if length is undefined (one byte: 0x80)
// returns -3 if too big length was found (max TLV size is 2^32 - 1)
inline int32 getTLVLength(const ByteArray & data, uint64 & tagNo, int & tagLen, int & lengthSize)
{
    const int64 valueLen = impl::decodeTLV((const uint8 *)data.constCharPtr(), data.size(), tagNo, tagLen, lengthSize);
    if (valueLen < 0) return valueLen;
    if (valueLen > (int64)0x7FFFFFFF - tagLen - lengthSize) return -3;
    if ((int64)data.size() < tagLen + lengthSize + valueLen) return -1;
    return valueLen;
}

inline ByteArray emptyTag(uint64 tagNo)
{
    const uint8 tagLen = impl::calcTagLen(tagNo);
    ByteArray rv(tagLen + 1, '\0');
    impl::writeTagBytes((uint8 *)rv.constData(), tagNo, false, tagLen);
    return rv;
}

template<typename T>
inline ByteArray toByteArray(const T & v, uint64 tagNo = 1)
{
    ByteArray rv;
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
inline T fromByteArray(const ByteArray & data, int tagLen, int lengthSize, int32 valueLen)
{
    if (valueLen == 0 && lengthSize == 2) return T();
    T retval;
    impl::BERDeserializerBase * dummy = 0;
    impl::deserializeBER(retval, (const uint8 *)data.constCharPtr() + tagLen + lengthSize, valueLen, *dummy);
    return retval;
}

template<typename T>
inline T fromByteArray(const ByteArray & data)
{
    uint64 tag = 0;
    int tagLen = 0;
    int lengthSize = 0;
    const int64 valueLen = getTLVLength(data, tag, tagLen, lengthSize);
    if (valueLen < 0) return T();

    if (valueLen == 0 && lengthSize == 2) return T();
    T retval;
    impl::BERDeserializerBase * dummy = 0;
    impl::deserializeBER(retval, (const uint8 *)data.constCharPtr() + tagLen + lengthSize, valueLen, *dummy);
    return retval;
}

template<typename... P, typename F>
inline void readAndCall(BERDeserializer & deser, F func)
{
    impl::ReadAndCall<P...>()(deser, func);
}

template<typename... P, typename F>
inline void readAndCall(const ByteArray & data, F func)
{
    BERDeserializer deser(data);
    impl::ReadAndCall<P...>()(deser, func);
}

} // namespace

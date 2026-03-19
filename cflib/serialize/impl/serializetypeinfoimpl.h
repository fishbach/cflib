/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>
#include <cflib/serialize/serializetypeinfo.h>

namespace cflib { namespace serialize { namespace impl {

class SerializeTypeInfoImpl : public SerializeTypeInfo {};

template<typename T>
inline SerializeTypeInfo fromType()
{
    SerializeTypeInfoImpl retval;
    serializeTypeInfo(retval, (T *)0);
    return retval;
}


// ============================================================================
// basic types
// ============================================================================

#define DO_SERIALIZE_TYPE_INFO(T, N) \
    inline void serializeTypeInfo(SerializeTypeInfoImpl & si, T *) { \
        si.type = SerializeTypeInfo::Basic; \
        si.typeName = #N; \
    } \

DO_SERIALIZE_TYPE_INFO(bool,    bool)
DO_SERIALIZE_TYPE_INFO(cfint8,   int8)
DO_SERIALIZE_TYPE_INFO(cfuint8,  uint8)
DO_SERIALIZE_TYPE_INFO(cfint16,  int16)
DO_SERIALIZE_TYPE_INFO(cfuint16, uint16)
DO_SERIALIZE_TYPE_INFO(cfint32,  int32)
DO_SERIALIZE_TYPE_INFO(cfuint32, uint32)
DO_SERIALIZE_TYPE_INFO(cfint64,  int64)
DO_SERIALIZE_TYPE_INFO(cfuint64, uint64)

DO_SERIALIZE_TYPE_INFO(float,       float32)
DO_SERIALIZE_TYPE_INFO(double,      float64)
DO_SERIALIZE_TYPE_INFO(long double, float128)

DO_SERIALIZE_TYPE_INFO(ByteArray, ByteArray)
DO_SERIALIZE_TYPE_INFO(String,    String)
DO_SERIALIZE_TYPE_INFO(CFDateTime,  DateTime)

template<typename T>
inline void serializeTypeInfo(SerializeTypeInfoImpl & si, CFFlags<T> *)
{
    serializeTypeInfo(si, (int *)0);
}


// ============================================================================
// container types
// ============================================================================

template<typename T1, typename T2>
inline void serializeTypeInfo(SerializeTypeInfoImpl & si, CFPair<T1, T2> *)
{
    SerializeTypeInfoImpl si1;
    serializeTypeInfo(si1, (T1 *)0);
    SerializeTypeInfoImpl si2;
    serializeTypeInfo(si2, (T2 *)0);
    si.type = SerializeTypeInfo::Container;
    si.typeName = String("Pair<") + si1.typeName + "," + si2.typeName + ">";
    si.bases.push_back(si1);
    si.bases.push_back(si2);
}

template<typename T>
inline void serializeTypeInfo(SerializeTypeInfoImpl & si, CFList<T> *)
{
    SerializeTypeInfoImpl si1;
    serializeTypeInfo(si1, (T *)0);
    si.type = SerializeTypeInfo::Container;
    si.typeName = String("List<") + si1.typeName + ">";
    si.bases.push_back(si1);
}

template<typename T>
inline void serializeTypeInfo(SerializeTypeInfoImpl & si, CFSet<T> *)
{
    serializeTypeInfo(si, (CFList<T> *)0);
}

template<typename Key, typename T>
inline void serializeTypeInfo(SerializeTypeInfoImpl & si, CFMap<Key, T> *)
{
    SerializeTypeInfoImpl si1;
    serializeTypeInfo(si1, (Key *)0);
    SerializeTypeInfoImpl si2;
    serializeTypeInfo(si2, (T *)0);
    si.type = SerializeTypeInfo::Container;
    si.typeName = String("Map<") + si1.typeName + "," + si2.typeName + ">";
    si.bases.push_back(si1);
    si.bases.push_back(si2);
}

template<typename Key, typename T>
inline void serializeTypeInfo(SerializeTypeInfoImpl & si, CFHash<Key, T> *)
{
    serializeTypeInfo(si, (CFMap<Key, T> *)0);
}


// ============================================================================
// classes
// ============================================================================

template<typename T>
inline void serializeTypeInfo(SerializeTypeInfoImpl & si, T *)
{
    (SerializeTypeInfo &)si = T::serializeTypeInfo();
}


// ============================================================================
// dynamic classes
// ============================================================================

template<typename T>
inline void serializeTypeInfo(SerializeTypeInfoImpl & si, CFSharedPtr<T> *)
{
    (SerializeTypeInfo &)si = T::serializeTypeInfo();
}

}}}    // namespace

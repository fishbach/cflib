/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

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
DO_SERIALIZE_TYPE_INFO(qint8,   int8)
DO_SERIALIZE_TYPE_INFO(quint8,  uint8)
DO_SERIALIZE_TYPE_INFO(qint16,  int16)
DO_SERIALIZE_TYPE_INFO(quint16, uint16)
DO_SERIALIZE_TYPE_INFO(qint32,  int32)
DO_SERIALIZE_TYPE_INFO(quint32, uint32)
DO_SERIALIZE_TYPE_INFO(qint64,  int64)
DO_SERIALIZE_TYPE_INFO(quint64, uint64)

DO_SERIALIZE_TYPE_INFO(float,       float32)
DO_SERIALIZE_TYPE_INFO(double,      float64)
DO_SERIALIZE_TYPE_INFO(long double, float128)

DO_SERIALIZE_TYPE_INFO(QByteArray, ByteArray)
DO_SERIALIZE_TYPE_INFO(QString,    String)
DO_SERIALIZE_TYPE_INFO(QChar,      Char)
DO_SERIALIZE_TYPE_INFO(QDateTime,  DateTime)

template<typename T>
inline void serializeTypeInfo(SerializeTypeInfoImpl & si, QFlags<T> *)
{
	serializeTypeInfo(si, (int *)0);
}


// ============================================================================
// container types
// ============================================================================

template<typename T1, typename T2>
inline void serializeTypeInfo(SerializeTypeInfoImpl & si, QPair<T1, T2> *)
{
	SerializeTypeInfoImpl si1;
	serializeTypeInfo(si1, (T1 *)0);
	SerializeTypeInfoImpl si2;
	serializeTypeInfo(si2, (T2 *)0);
	si.type = SerializeTypeInfo::Container;
	si.typeName = "Pair<" + si1.typeName + "," + si2.typeName + ">";
	si.bases << si1 << si2;
}

template<typename T>
inline void serializeTypeInfo(SerializeTypeInfoImpl & si, QList<T> *)
{
	SerializeTypeInfoImpl si1;
	serializeTypeInfo(si1, (T *)0);
	si.type = SerializeTypeInfo::Container;
	si.typeName = "List<" + si1.typeName + ">";
	si.bases << si1;
}

inline void serializeTypeInfo(SerializeTypeInfoImpl & si, QStringList *)
{
	serializeTypeInfo(si, (QList<QString> *)0);
}

template<typename T>
inline void serializeTypeInfo(SerializeTypeInfoImpl & si, QLinkedList<T> *)
{
	serializeTypeInfo(si, (QList<T> *)0);
}

template<typename T>
inline void serializeTypeInfo(SerializeTypeInfoImpl & si, QVector<T> *)
{
	serializeTypeInfo(si, (QList<T> *)0);
}

template<typename T>
inline void serializeTypeInfo(SerializeTypeInfoImpl & si, QSet<T> *)
{
	serializeTypeInfo(si, (QList<T> *)0);
}

template<typename Key, typename T>
inline void serializeTypeInfo(SerializeTypeInfoImpl & si, QMap<Key, T> *)
{
	SerializeTypeInfoImpl si1;
	serializeTypeInfo(si1, (Key *)0);
	SerializeTypeInfoImpl si2;
	serializeTypeInfo(si2, (T *)0);
	si.type = SerializeTypeInfo::Container;
	si.typeName = "Map<" + si1.typeName + "," + si2.typeName + ">";
	si.bases << si1 << si2;
}

template<typename Key, typename T>
inline void serializeTypeInfo(SerializeTypeInfoImpl & si, QHash<Key, T> *)
{
	serializeTypeInfo(si, (QMap<Key, T> *)0);
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
inline void serializeTypeInfo(SerializeTypeInfoImpl & si, QSharedPointer<T> *)
{
	(SerializeTypeInfo &)si = T::serializeTypeInfo();
}

}}}	// namespace

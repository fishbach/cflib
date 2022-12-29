/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/serialize/impl/registerclass.h>
#include <cflib/serialize/impl/serializebaseber.h>

namespace cflib { namespace serialize { namespace impl {

// ============================================================================
// integer
// ============================================================================

template<typename T>
inline void serializeBERInt(T v, quint64 tagNo, QByteArray & data, bool isMaxUInt = false)
{
	if (v == 0) { writeNull(data, tagNo); return; }

	quint8 len = isMaxUInt ? 9 : minSizeOfInt(v);

	const quint8 tagLen = calcTagLen(tagNo);
	const int size = data.size();
	data.resize(size + tagLen + 1 + len);
	quint8 * p = (quint8 *)data.constData() + size;	// constData for performance

	// write tag
	writeTagBytes(p, tagNo, false, tagLen);
	p += tagLen;

	// write len
	*p = len;

	// write value
	p += len;
	*p = (quint8)v;
	while (--len > 0) {
		if (sizeof(v) == 1) {
			*(--p) = 0;
		} else {
			v >>= 8;
			*(--p) = (quint8)v;
		}
	}
}

template<>
inline void serializeBERInt(bool v, quint64 tagNo, QByteArray & data, bool)
{
	if (!v) { writeNull(data, tagNo); return; }

	const quint8 tagLen = calcTagLen(tagNo);
	const int size = data.size();
	data.resize(size + tagLen + 2);
	quint8 * p = (quint8 *)data.constData() + size;	// constData for performance

	// write tag
	writeTagBytes(p, tagNo, false, tagLen);

	// write len
	p[tagLen] = 1;

	// write value
	p[tagLen + 1] = 1;
}

inline void serializeBERInt(quint64 v, quint64 tagNo, QByteArray & data)
{
	serializeBERInt<quint64>(v, tagNo, data, (v >> 63));
}

template<typename T>
inline void deserializeBERInt(T & v, const quint8 * data, int len)
{
	if (len < 1) {
		v = 0;
		return;
	}
	const quint8 * b = (const quint8 *)data;
	v = *b;
	if (v & 0x80) v = ((T)-1 & ~0xFF) | v;
	while (--len > 0) v = (v << 8) | *(++b);
}

template<>
inline void deserializeBERInt(bool & v, const quint8 * data, int len)
{
	v = len > 0 && (quint8)*data == 1;
}

// ----------------------------------------------------------------------------

#define DO_SERIALIZE_BER(typ) \
	inline void serializeBER(typ val, quint64 tag, QByteArray & data, BERSerializerBase &) { \
		serializeBERInt(val, tag, data); } \
	inline void deserializeBER(typ & val, const quint8 * data, int len, BERDeserializerBase &) { \
		deserializeBERInt(val, data, len); } \

DO_SERIALIZE_BER(bool   )
DO_SERIALIZE_BER(qint8  )
DO_SERIALIZE_BER(quint8 )
DO_SERIALIZE_BER(qint16 )
DO_SERIALIZE_BER(quint16)
DO_SERIALIZE_BER(qint32 )
DO_SERIALIZE_BER(quint32)
DO_SERIALIZE_BER(qint64 )
DO_SERIALIZE_BER(quint64)


// ============================================================================
// float
// ============================================================================

inline void serializeBER(float d, quint64 tagNo, QByteArray & data, BERSerializerBase &)
{
	if (d == 0) { writeNull(data, tagNo); return; }
	const quint8 tagLen = calcTagLen(tagNo);
	const int oldSize = data.size();
	data.resize(oldSize + tagLen + 5);
	quint8 * pos = (quint8 *)data.constData() + oldSize;
	writeTagBytes(pos, tagNo, false, tagLen);
	pos += tagLen;
	*(pos++) = 4;
	memcpy(pos, (const char *)&d, 4);
}

inline void deserializeBER(float & d, const quint8 * data, int len, BERDeserializerBase &)
{
	d = len != 4 ? 0 : *((const float *)data);
}

inline void serializeBER(double d, quint64 tagNo, QByteArray & data, BERSerializerBase &)
{
	if (d == 0) { writeNull(data, tagNo); return; }
	const quint8 tagLen = calcTagLen(tagNo);
	const int oldSize = data.size();
	data.resize(oldSize + tagLen + 9);
	quint8 * pos = (quint8 *)data.constData() + oldSize;
	writeTagBytes(pos, tagNo, false, tagLen);
	pos += tagLen;
	*(pos++) = 8;
	memcpy(pos, (const char *)&d, 8);
}

inline void deserializeBER(double & d, const quint8 * data, int len, BERDeserializerBase &)
{
	d = len != 8 ? 0 : *((const double *)data);
}

inline void serializeBER(long double d, quint64 tagNo, QByteArray & data, BERSerializerBase &)
{
	if (d == 0) { writeNull(data, tagNo); return; }
	const quint8 tagLen = calcTagLen(tagNo);
	const int oldSize = data.size();
	data.resize(oldSize + tagLen + 17);
	quint8 * pos = (quint8 *)data.constData() + oldSize;
	writeTagBytes(pos, tagNo, false, tagLen);
	pos += tagLen;
	*(pos++) = 16;
	memcpy(pos, (const char *)&d, 16);
}

inline void deserializeBER(long double & d, const quint8 * data, int len, BERDeserializerBase &)
{
	d = len != 16 ? 0 : *((const long double *)data);
}


// ============================================================================
// strings
// ============================================================================

// ----------------------------------------------------------------------------
// QByteArray
// ----------------------------------------------------------------------------

inline void serializeBER(const QByteArray & ba, quint64 tagNo, QByteArray & data, BERSerializerBase &)
{
	if (ba.isNull())  { writeNull(data, tagNo); return; }
	if (ba.isEmpty()) { writeZero(data, tagNo); return; }

	const quint8 tagLen = calcTagLen(tagNo);
	const quint8 lengthSize = calcBERlengthSize(ba.size());
	const int oldSize = data.size();
	data.resize(oldSize + tagLen + lengthSize + ba.size());
	quint8 * pos = (quint8 *)data.constData() + oldSize;
	writeTagBytes(pos, tagNo, false, tagLen);
	writeLenBytes(pos + tagLen, ba.size(), lengthSize);
	memcpy(pos + tagLen + lengthSize, ba.constData(), ba.size());
}

inline void deserializeBER(QByteArray & ba, const quint8 * data, int len, BERDeserializerBase &)
{
	ba = QByteArray((const char *)data, len);
}

// ----------------------------------------------------------------------------
// QString
// ----------------------------------------------------------------------------

inline void serializeBER(const QString & str, quint64 tagNo, QByteArray & data, BERSerializerBase & ser)
{
	if (str.isNull())  { writeNull(data, tagNo); return; }
	if (str.isEmpty()) { writeZero(data, tagNo); return; }
	serializeBER(str.toUtf8(), tagNo, data, ser);
}

inline void deserializeBER(QString & str, const quint8 * data, int len, BERDeserializerBase &)
{
	str = QString::fromUtf8((const char *)data, len);
}

// ----------------------------------------------------------------------------
// const char * (use QByteArray for deserialization)
// ----------------------------------------------------------------------------

inline void serializeBER(const char * str, quint64 tagNo, QByteArray & data, BERSerializerBase &)
{
	if (str == 0) { writeNull(data, tagNo); return; }

	const qint64 len = strlen(str);
	if (len == 0) { writeZero(data, tagNo); return; }

	const quint8 tagLen = calcTagLen(tagNo);
	const quint8 lengthSize = calcBERlengthSize(len);
	const int oldSize = data.size();
	data.resize(oldSize + tagLen + lengthSize + len);
	quint8 * pos = (quint8 *)data.constData() + oldSize;
	writeTagBytes(pos, tagNo, false, tagLen);
	writeLenBytes(pos + tagLen, len, lengthSize);
	memcpy(pos + tagLen + lengthSize, str, len);
}

// ----------------------------------------------------------------------------
// QChar
// ----------------------------------------------------------------------------

inline void serializeBER(const QChar & c, quint64 tagNo, QByteArray & data, BERSerializerBase &)
{
	if (c.isNull())  { writeNull(data, tagNo); return; }
	serializeBERInt(c.unicode(), tagNo, data);
}

inline void deserializeBER(QChar & c, const quint8 * data, int len, BERDeserializerBase &)
{
	ushort unicode;
	deserializeBERInt(unicode, data, len);
	c = unicode;
}


// ============================================================================
// Qt classes
// ============================================================================

// ----------------------------------------------------------------------------
// QDateTime
// ----------------------------------------------------------------------------

inline void serializeBER(const QDateTime & dt, quint64 tagNo, QByteArray & data, BERSerializerBase &)
{
	if (dt.isNull()) { writeNull(data, tagNo); return; }
	serializeBERInt(dt.toMSecsSinceEpoch(), tagNo, data);
}

inline void deserializeBER(QDateTime & dt, const quint8 * data, int len, BERDeserializerBase &)
{
	qint64 msec;
	deserializeBERInt(msec, data, len);
	dt.setTimeSpec(Qt::UTC);
	dt.setMSecsSinceEpoch(msec);
}

// ----------------------------------------------------------------------------
// QFlags
// ----------------------------------------------------------------------------

template<typename T>
inline void serializeBER(const QFlags<T> & fl, quint64 tagNo, QByteArray & data, BERSerializerBase &)
{
	serializeBERInt((int)fl, tagNo, data);
}

template<typename T>
inline void deserializeBER(QFlags<T> & fl, const quint8 * data, int len, BERDeserializerBase &)
{
	int flags;
	deserializeBERInt(flags, data, len);
	fl = (T)flags;
}


// ============================================================================
// Qt container types
// ============================================================================

// see qdatastream.h

// ----------------------------------------------------------------------------
// QPair
// ----------------------------------------------------------------------------

template<typename T1, typename T2>
inline void serializeBER(const QPair<T1, T2> & cl, quint64 tagNo, QByteArray & data, BERSerializerBase &)
{
	TLWriter tlw(data, tagNo);
	BERSerializerBase ser(data);
	ser << cl.first << cl.second;
}

template<typename T1, typename T2>
inline void deserializeBER(QPair<T1, T2> & cl, const quint8 * data, int len, BERDeserializerBase &)
{
	BERDeserializerBase ser(data, len);
	ser >> cl.first >> cl.second;
}

// ----------------------------------------------------------------------------
// QList
// ----------------------------------------------------------------------------

template<typename T>
inline void serializeBER(const QList<T> & cl, quint64 tagNo, QByteArray & data, BERSerializerBase &)
{
	TLWriter tlw(data, tagNo);
	BERSerializerBase ser(data, true);
	for (typename QList<T>::const_iterator it = cl.begin() ; it != cl.end() ; ++it) ser << *it;
}

template<typename T>
inline void deserializeBER(QList<T> & cl, const quint8 * data, int len, BERDeserializerBase &)
{
	BERDeserializerBase ser(data, len, true);
	cl.clear();
	while (ser.isAnyAvailable()) {
		T el; ser >> el;
		cl.append(el);
	}
}

// ----------------------------------------------------------------------------
// QStringList
// ----------------------------------------------------------------------------

inline void serializeBER(const QStringList & cl, quint64 tagNo, QByteArray & data, BERSerializerBase & ser)
{
	serializeBER((const QList<QString> &)cl, tagNo, data, ser);
}

inline void deserializeBER(QStringList & cl, const quint8 * data, int len, BERDeserializerBase & ser)
{
	deserializeBER((QList<QString> &)cl, data, len, ser);
}

// ----------------------------------------------------------------------------
// QLinkedList
// ----------------------------------------------------------------------------

template<typename T>
inline void serializeBER(const QLinkedList<T> & cl, quint64 tagNo, QByteArray & data, BERSerializerBase &)
{
	TLWriter tlw(data, tagNo);
	BERSerializerBase ser(data, true);
	for (typename QLinkedList<T>::const_iterator it = cl.begin() ; it != cl.end() ; ++it) ser << *it;
}

template<typename T>
inline void deserializeBER(QLinkedList<T> & cl, const quint8 * data, int len, BERDeserializerBase &)
{
	BERDeserializerBase ser(data, len, true);
	cl.clear();
	while (ser.isAnyAvailable()) {
		T el; ser >> el;
		cl.append(el);
	}
}

// ----------------------------------------------------------------------------
// QVector
// ----------------------------------------------------------------------------

template<typename T>
inline void serializeBER(const QVector<T> & cl, quint64 tagNo, QByteArray & data, BERSerializerBase &)
{
	TLWriter tlw(data, tagNo);
	BERSerializerBase ser(data, true);
	for (typename QVector<T>::const_iterator it = cl.begin() ; it != cl.end() ; ++it) ser << *it;
}

template<typename T>
inline void deserializeBER(QVector<T> & cl, const quint8 * data, int len, BERDeserializerBase &)
{
	BERDeserializerBase ser(data, len, true);
	cl.clear();
	while (ser.isAnyAvailable()) {
		T el; ser >> el;
		cl.append(el);
	}
}

// ----------------------------------------------------------------------------
// QSet
// ----------------------------------------------------------------------------

template<typename T>
inline void serializeBER(const QSet<T> & cl, quint64 tagNo, QByteArray & data, BERSerializerBase &)
{
	TLWriter tlw(data, tagNo);
	BERSerializerBase ser(data, true);
	if (cl.isEmpty()) return;
	for (typename QSet<T>::const_iterator it = cl.constBegin() ; it != cl.constEnd() ; ++it) ser << *it;
}

template<typename T>
inline void deserializeBER(QSet<T> & cl, const quint8 * data, int len, BERDeserializerBase &)
{
	BERDeserializerBase ser(data, len, true);
	cl.clear();
	while (ser.isAnyAvailable()) {
		T el; ser >> el;
		cl << el;
	}
}

// ----------------------------------------------------------------------------
// QHash
// ----------------------------------------------------------------------------

template<typename Key, typename T>
inline void serializeBER(const QHash<Key, T> & cl, quint64 tagNo, QByteArray & data, BERSerializerBase &)
{
	TLWriter tlw(data, tagNo);
	BERSerializerBase ser(data, true);
	typename QHash<Key, T>::const_iterator it = cl.end();
	typename QHash<Key, T>::const_iterator begin = cl.begin();
	while (it != begin) {
		--it;
		ser << it.key() << it.value();
	}
}

template<typename Key, typename T>
inline void deserializeBER(QHash<Key, T> & cl, const quint8 * data, int len, BERDeserializerBase &)
{
	BERDeserializerBase ser(data, len, true);
	cl.clear();
	while (ser.isAnyAvailable()) {
		Key key;
		T value;
		ser >> key >> value;
		cl.insertMulti(key, value);
	}
}

// ----------------------------------------------------------------------------
// QMultiMap
// ----------------------------------------------------------------------------

template<typename Key, typename T>
inline void serializeBER(const QMultiMap<Key, T> & cl, quint64 tagNo, QByteArray & data, BERSerializerBase &)
{
	TLWriter tlw(data, tagNo);
	BERSerializerBase ser(data, true);
	typename QMultiMap<Key, T>::const_iterator it = cl.end();
	typename QMultiMap<Key, T>::const_iterator begin = cl.begin();
	while (it != begin) {
		--it;
		ser << it.key() << it.value();
	}
}

template<typename Key, typename T>
inline void deserializeBER(QMultiMap<Key, T> & cl, const quint8 * data, int len, BERDeserializerBase &)
{
	BERDeserializerBase ser(data, len, true);
	cl.clear();
	while (ser.isAnyAvailable()) {
		Key key;
		T value;
		ser >> key >> value;
		cl.insert(key, value);
	}
}


// ============================================================================
// custom classes
// ============================================================================

template<typename T>
inline void serializeBER(const T & cl, quint64 tagNo, QByteArray & data, BERSerializerBase &)
{
	TLWriter tlw(data, tagNo);
	BERSerializerBase ser(data);
	cl.serialize(ser);
}

template<typename T>
inline void deserializeBER(T & cl, const quint8 * data, int len, BERDeserializerBase &)
{
	BERDeserializerBase ser(data, len);
	cl.deserialize(ser);
}


// ============================================================================
// dynamic classes
// ============================================================================

template<typename T>
inline void serializeBER(const QSharedPointer<T> & cl, quint64 tagNo, QByteArray & data, BERSerializerBase &)
{
	if (cl.isNull()) return;

	TLWriter tlw(data, tagNo);
	BERSerializerBase ser(data);
	RegisterClassBase::serialize(cl, ser);
}

template<typename T>
inline void deserializeBER(QSharedPointer<T> & cl, const quint8 * data, int len, BERDeserializerBase &)
{
	RegisterClassBase::deserialize(cl, data, len);
}


}}}	// namespace

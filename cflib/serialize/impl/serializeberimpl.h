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

#include <cflib/serialize/impl/serializebaseber.h>

namespace cflib { namespace serialize { namespace impl {

// ============================================================================
// integer
// ============================================================================

template <typename T>
inline void serializeBERInt(T v, quint64 tag, quint8 tagLen, QByteArray & data, bool isMaxUInt = false)
{
	if (v == 0) { writeNull(data, tag, tagLen); return; }

	quint8 len = isMaxUInt ? 9 : minSizeOfInt(v);

	const int size = data.size();
	data.resize(size + tagLen + 1 + len);
	quint8 * p = (quint8 *)data.constData() + size;	// constData for performance

	// write tag
	writeTag(p, tag, tagLen);
	p += tagLen;

	// write len
	*p = len;

	// write value
	p += len;
	*p = (quint8)v;
	while (--len > 0) {
		v >>= 8;
		*(--p) = (quint8)v;
	}
}

template <>
inline void serializeBERInt(bool v, quint64 tag, quint8 tagLen, QByteArray & data, bool)
{
	if (!v) { writeNull(data, tag, tagLen); return; }

	const int size = data.size();
	data.resize(size + tagLen + 2);
	quint8 * p = (quint8 *)data.constData() + size;	// constData for performance

	// write tag
	writeTag(p, tag, tagLen);

	// write len
	p[tagLen] = 1;

	// write value
	p[tagLen + 1] = 1;
}

inline void serializeBERInt(quint64 v, quint64 tag, quint8 tagLen, QByteArray & data)
{
	serializeBERInt<quint64>(v, tag, tagLen, data, (v >> 63));
}

template <typename T>
inline void deserializeBERInt(T & v, const quint8 * data, int len)
{
	if (len < 1) {
		v = 0;
		return;
	}
	const quint8 * b = (const quint8 *)data;
	v = *b;
	if (v & 0x80) v = (-1 << 8) | v;
	while (--len > 0) v = (v << 8) | *(++b);
}

template <>
inline void deserializeBERInt(bool & v, const quint8 * data, int len)
{
	v = len > 0 && (quint8)*data == 1;
}

// ----------------------------------------------------------------------------

#define DO_SERIALIZE_BER(typ) \
	inline void serializeBER(typ val, quint64 tag, quint8 tagLen, QByteArray & data, BERSerializerBase &) { \
		serializeBERInt(val, tag, tagLen, data); } \
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
// strings
// ============================================================================

// ----------------------------------------------------------------------------
// QByteArray
// ----------------------------------------------------------------------------

inline void serializeBER(const QByteArray & ba, quint64 tag, quint8 tagLen, QByteArray & data, BERSerializerBase &)
{
	if (ba.isNull())  { writeNull(data, tag, tagLen); return; }
	if (ba.isEmpty()) { writeZero(data, tag, tagLen); return; }

	writeTag(data, tag, tagLen);
	QByteArray lenBytes;
	data += (char)encodeBERLength(ba.size(), lenBytes);
	if (!lenBytes.isEmpty()) data += lenBytes;
	data += ba;
}

inline void deserializeBER(QByteArray & ba, const quint8 * data, int len, BERDeserializerBase &)
{
	ba = QByteArray((const char *)data, len);
}

// ----------------------------------------------------------------------------
// QString
// ----------------------------------------------------------------------------

inline void serializeBER(const QString & str, quint64 tag, quint8 tagLen, QByteArray & data, BERSerializerBase & ser)
{
	if (str.isNull())  { writeNull(data, tag, tagLen); return; }
	if (str.isEmpty()) { writeZero(data, tag, tagLen); return; }
	serializeBER(str.toUtf8(), tag, tagLen, data, ser);
}

inline void deserializeBER(QString & str, const quint8 * data, int len, BERDeserializerBase &)
{
	str = QString::fromUtf8((const char *)data, len);
}

// ----------------------------------------------------------------------------
// const char * (use QByteArray for deserialization)
// ----------------------------------------------------------------------------

inline void serializeBER(const char * str, quint64 tag, quint8 tagLen, QByteArray & data, BERSerializerBase &)
{
	if (str == 0) { writeNull(data, tag, tagLen); return; }

	const qint64 len = strlen(str);
	if (len == 0) { writeZero(data, tag, tagLen); return; }

	QByteArray lenBytes;
	const quint8 fb = encodeBERLength(len, lenBytes);

	const int size = data.size();
	data.resize(size + 2 + lenBytes.size() + len);
	quint8 * p = (quint8 *)data.constData() + size;	// constData for performance

	// write tag
	writeTag(p, tag, tagLen);
	p += tagLen;

	*p = fb;
	memcpy(++p, lenBytes.constData(), lenBytes.size());
	p += lenBytes.size();
	memcpy(p, str, len);
}

// ----------------------------------------------------------------------------
// QChar
// ----------------------------------------------------------------------------

inline void serializeBER(const QChar & c, quint64 tag, quint8 tagLen, QByteArray & data, BERSerializerBase &)
{
	if (c.isNull())  { writeNull(data, tag, tagLen); return; }
	serializeBERInt(c.unicode(), tag, tagLen, data);
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

inline void serializeBER(const QDateTime & dt, quint64 tag, quint8 tagLen, QByteArray & data, BERSerializerBase &)
{
	if (dt.isNull()) { writeNull(data, tag, tagLen); return; }
	serializeBERInt(dt.toMSecsSinceEpoch(), tag, tagLen, data);
}

inline void deserializeBER(QDateTime & dt, const quint8 * data, int len, BERDeserializerBase &)
{
	qint64 msec;
	deserializeBERInt(msec, data, len);
	dt.setMSecsSinceEpoch(msec);
}

// ----------------------------------------------------------------------------
// QFlags
// ----------------------------------------------------------------------------

template <typename T>
inline void serializeBER(const QFlags<T> & fl, quint64 tag, quint8 tagLen, QByteArray & data, BERSerializerBase &)
{
	serializeBERInt((int)fl, tag, tagLen, data);
}

template <typename T>
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

template <typename T1, typename T2>
inline void serializeBER(const QPair<T1, T2> & cl, quint64 tag, quint8 tagLen, QByteArray & data, BERSerializerBase &)
{
	TLWriter tlw(data, tag, tagLen);
	BERSerializerBase ser(data);
	ser << cl.first << cl.second;
}

template <typename T1, typename T2>
inline void deserializeBER(QPair<T1, T2> & cl, const quint8 * data, int len, BERDeserializerBase &)
{
	BERDeserializerBase ser(data, len);
	ser >> cl.first >> cl.second;
}

// ----------------------------------------------------------------------------
// QList
// ----------------------------------------------------------------------------

template <typename T>
inline void serializeBER(const QList<T> & cl, quint64 tag, quint8 tagLen, QByteArray & data, BERSerializerBase &)
{
	TLWriter tlw(data, tag, tagLen);
	BERSerializerBase ser(data, true);
	for (typename QList<T>::const_iterator it = cl.begin() ; it != cl.end() ; ++it) ser << *it;
}

template <typename T>
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

inline void serializeBER(const QStringList & cl, quint64 tag, quint8 tagLen, QByteArray & data, BERSerializerBase & ser)
{
	serializeBER((const QList<QString> &)cl, tag, tagLen, data, ser);
}

inline void deserializeBER(QStringList & cl, const quint8 * data, int len, BERDeserializerBase & ser)
{
	deserializeBER((QList<QString> &)cl, data, len, ser);
}

// ----------------------------------------------------------------------------
// QLinkedList
// ----------------------------------------------------------------------------

template <typename T>
inline void serializeBER(const QLinkedList<T> & cl, quint64 tag, quint8 tagLen, QByteArray & data, BERSerializerBase &)
{
	TLWriter tlw(data, tag, tagLen);
	BERSerializerBase ser(data, true);
	for (typename QLinkedList<T>::const_iterator it = cl.begin() ; it != cl.end() ; ++it) ser << *it;
}

template <typename T>
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

template <typename T>
inline void serializeBER(const QVector<T> & cl, quint64 tag, quint8 tagLen, QByteArray & data, BERSerializerBase &)
{
	TLWriter tlw(data, tag, tagLen);
	BERSerializerBase ser(data, true);
	ser << cl.size();
	for (typename QVector<T>::const_iterator it = cl.begin() ; it != cl.end() ; ++it) ser << *it;
}

template <typename T>
inline void deserializeBER(QVector<T> & cl, const quint8 * data, int len, BERDeserializerBase &)
{
	BERDeserializerBase ser(data, len, true);
	cl.clear();
	quint32 size; ser >> size;
	cl.resize(size);
	for (quint32 i = 0 ; i < size ; ++i) {
		T el; ser >> el;
		cl[i] = el;
	}
}

// ----------------------------------------------------------------------------
// QSet
// ----------------------------------------------------------------------------

template <typename T>
inline void serializeBER(const QSet<T> & cl, quint64 tag, quint8 tagLen, QByteArray & data, BERSerializerBase &)
{
	TLWriter tlw(data, tag, tagLen);
	BERSerializerBase ser(data, true);
	for (typename QSet<T>::const_iterator it = cl.constBegin() ; it != cl.constEnd() ; ++it) ser << *it;
}

template <typename T>
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

template <typename Key, typename T>
inline void serializeBER(const QHash<Key, T> & cl, quint64 tag, quint8 tagLen, QByteArray & data, BERSerializerBase &)
{
	TLWriter tlw(data, tag, tagLen);
	BERSerializerBase ser(data, true);
	typename QHash<Key, T>::const_iterator it = cl.end();
	typename QHash<Key, T>::const_iterator begin = cl.begin();
	while (it != begin) {
		--it;
		ser << it.key() << it.value();
	}
}

template <typename Key, typename T>
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
// QMap
// ----------------------------------------------------------------------------

template <typename Key, typename T>
inline void serializeBER(const QMap<Key, T> & cl, quint64 tag, quint8 tagLen, QByteArray & data, BERSerializerBase &)
{
	TLWriter tlw(data, tag, tagLen);
	BERSerializerBase ser(data, true);
	typename QMap<Key, T>::const_iterator it = cl.end();
	typename QMap<Key, T>::const_iterator begin = cl.begin();
	while (it != begin) {
		--it;
		ser << it.key() << it.value();
	}
}

template <typename Key, typename T>
inline void deserializeBER(QMap<Key, T> & cl, const quint8 * data, int len, BERDeserializerBase &)
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


// ============================================================================
// custom classes
// ============================================================================

template <typename T>
inline void serializeBER(const T & cl, quint64 tag, quint8 tagLen, QByteArray & data, BERSerializerBase &)
{
	TLWriter tlw(data, tag, tagLen);
	BERSerializerBase ser(data);
	cl.serialize(ser);
}

template <typename T>
inline void deserializeBER(T & cl, const quint8 * data, int len, BERDeserializerBase &)
{
	BERDeserializerBase ser(data, len);
	cl.deserialize(ser);
}


}}}	// namespace

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

#include <cflib/serialize/impl/serializebasejs.h>

namespace cflib { namespace serialize { namespace impl {

// ============================================================================
// integer
// ============================================================================

template <typename T>
inline void serializeJSInt(T v, QByteArray & data)
{
	if (v == 0) return;
	data += QByteArray::number(v);
}

template <typename T>
inline void deserializeJSInt(T & v, const quint8 * data, int len)
{
	v = 0;
	T deci = 1;
	const quint8 * p = data + len;
	while (--len >= 0) {
		const quint8 b = *(--p);
		if (len == 0 && b == (quint8)'-') {
			v *= -1;
			return;
		} else {
			v += (b - 48) * deci;
			deci *= 10;
		}
	}
}

template <>
inline void deserializeJSInt(bool & v, const quint8 * data, int len)
{
	v = len > 0 && (char)*data == '1';
}

// ----------------------------------------------------------------------------

#define DO_SERIALIZE_JS(type) \
	inline void serializeJS(type val, QByteArray & data, JSSerializerBase &) { \
		serializeJSInt(val, data); } \
	inline void deserializeJS(type & val, const quint8 * data, int len, JSDeserializerBase &) { \
		deserializeJSInt(val, data, len); } \

DO_SERIALIZE_JS(bool   )
DO_SERIALIZE_JS(qint8  )
DO_SERIALIZE_JS(quint8 )
DO_SERIALIZE_JS(qint16 )
DO_SERIALIZE_JS(quint16)
DO_SERIALIZE_JS(qint32 )
DO_SERIALIZE_JS(quint32)
DO_SERIALIZE_JS(qint64 )
DO_SERIALIZE_JS(quint64)


// ============================================================================
// strings
// ============================================================================

// ----------------------------------------------------------------------------
// QByteArray
// ----------------------------------------------------------------------------

inline void serializeJS(const QByteArray & baRef, QByteArray & data, JSSerializerBase &)
{
	if (baRef.isNull()) return;
	if (baRef.isEmpty()) { data += "''"; return; }

	data += '\'';
	QByteArray ba = baRef;
	ba
		.replace('\\', "\\\\")
		.replace('\'', "\\'")

		// We want to allow 'eval' in JavaScript (see ECMA-262).
		.replace('\n', "\\n")
		.replace('\r', "\\r")
		.replace("\xE2\x80\xA8", "\\u2028")		// Line separator <LS>
		.replace("\xE2\x80\xA9", "\\u2029");	// Paragraph separator <PS>

	data += ba;
	data += '\'';
}

inline void deserializeJS(QByteArray & ba, const quint8 * data, int len, JSDeserializerBase &)
{
	if (len < 2 || *data != (quint8)'\'' || data[len - 1] != (quint8)'\'') {
		ba = QByteArray();
		return;
	}
	len -= 2;

	ba.resize(0);
	bool isEsc = false;
	while (--len >= 0) {
		const char c = (char)*(++data);

		if (isEsc) {
			isEsc = false;
			if      (c == '\\') ba += '\\';
			else if (c == '\'') ba += '\'';
			else if (c == 'n')  ba += '\n';
			else if (c == 'r')  ba += '\r';
			else {
				if (c == 'u' && len >= 4) {
					const char * d = (const char *)data;
					if (d[1] == '2' && d[2] == '0' && d[3] == '2') {
						if (d[4] == '8') {
							ba += "\xE2\x80\xA8";
							len  -= 4;
							data += 4;
							continue;
						} else if (d[4] == '9') {
							ba += "\xE2\x80\xA9";
							len  -= 4;
							data += 4;
							continue;
						}
					}
				}
				ba += '\\';
				ba += c;
			}
		} else if (c == '\\') isEsc = true;
		else ba += c;
	}
}

// ----------------------------------------------------------------------------
// QString
// ----------------------------------------------------------------------------

inline void serializeJS(const QString & str, QByteArray & data, JSSerializerBase & ser)
{
	serializeJS(str.toUtf8(), data, ser);
}

inline void deserializeJS(QString & str, const quint8 * data, int len, JSDeserializerBase & ser)
{
	QByteArray ba;
	deserializeJS(ba, data, len, ser);
	str = QString::fromUtf8(ba);
}

// ----------------------------------------------------------------------------
// const char * (use QByteArray for deserialization)
// ----------------------------------------------------------------------------

inline void serializeJS(const char * str, QByteArray & data, JSSerializerBase & ser)
{
	if (str == 0) return;
	serializeJS(QByteArray(str), data, ser);
}

// ----------------------------------------------------------------------------
// QChar
// ----------------------------------------------------------------------------

inline void serializeJS(const QChar & c, QByteArray & data, JSSerializerBase &)
{
	if (c.isNull()) return;
	serializeJSInt(c.unicode(), data);
}

inline void deserializeJS(QChar & c, const quint8 * data, int len, JSDeserializerBase &)
{
	ushort unicode;
	deserializeJSInt(unicode, data, len);
	c = unicode;
}


// ============================================================================
// Qt classes
// ============================================================================

// ----------------------------------------------------------------------------
// QDateTime
// ----------------------------------------------------------------------------

inline void serializeJS(const QDateTime & dt, QByteArray & data, JSSerializerBase &)
{
	if (dt.isNull()) return;
	serializeJSInt(dt.toMSecsSinceEpoch(), data);
}

inline void deserializeJS(QDateTime & dt, const quint8 * data, int len, JSDeserializerBase &)
{
	qint64 msec;
	deserializeJSInt(msec, data, len);
	dt.setTimeSpec(Qt::UTC);
	dt.setMSecsSinceEpoch(msec);
}

// ----------------------------------------------------------------------------
// QFlags
// ----------------------------------------------------------------------------

template <typename T>
inline void serializeJS(const QFlags<T> & fl, QByteArray & data, JSSerializerBase &)
{
	serializeJSInt((int)fl, data);
}

template <typename T>
inline void deserializeJS(QFlags<T> & fl, const quint8 * data, int len, JSDeserializerBase &)
{
	int flags;
	deserializeJSInt(flags, data, len);
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
inline void serializeJS(const QPair<T1, T2> & cl, QByteArray & data, JSSerializerBase &)
{
	JSSerializerBase ser(data);
	ser << cl.first << cl.second;
}

template <typename T1, typename T2>
inline void deserializeJS(QPair<T1, T2> & cl, const quint8 * data, int len, JSDeserializerBase &)
{
	JSDeserializerBase ser(data, len);
	ser >> cl.first >> cl.second;
}

// ----------------------------------------------------------------------------
// QList
// ----------------------------------------------------------------------------

template <typename T>
inline void serializeJS(const QList<T> & cl, QByteArray & data, JSSerializerBase &)
{
	JSSerializerBase ser(data);
	for (typename QList<T>::const_iterator it = cl.begin() ; it != cl.end() ; ++it) ser << *it;
}

template <typename T>
inline void deserializeJS(QList<T> & cl, const quint8 * data, int len, JSDeserializerBase &)
{
	JSDeserializerBase ser(data, len);
	cl.clear();
	while (ser.isAnyAvailable()) {
		T el; ser >> el;
		cl.append(el);
	}
}

// ----------------------------------------------------------------------------
// QStringList
// ----------------------------------------------------------------------------

inline void serializeJS(const QStringList & cl, QByteArray & data, JSSerializerBase & ser)
{
	serializeJS((const QList<QString> &)cl, data, ser);
}

inline void deserializeJS(QStringList & cl, const quint8 * data, int len, JSDeserializerBase & ser)
{
	deserializeJS((QList<QString> &)cl, data, len, ser);
}

// ----------------------------------------------------------------------------
// QLinkedList
// ----------------------------------------------------------------------------

template <typename T>
inline void serializeJS(const QLinkedList<T> & cl, QByteArray & data, JSSerializerBase &)
{
	JSSerializerBase ser(data);
	for (typename QLinkedList<T>::const_iterator it = cl.begin() ; it != cl.end() ; ++it) ser << *it;
}

template <typename T>
inline void deserializeJS(QLinkedList<T> & cl, const quint8 * data, int len, JSDeserializerBase &)
{
	JSDeserializerBase ser(data, len);
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
inline void serializeJS(const QVector<T> & cl, QByteArray & data, JSSerializerBase &)
{
	JSSerializerBase ser(data);
	for (typename QVector<T>::const_iterator it = cl.begin() ; it != cl.end() ; ++it) ser << *it;
}

template <typename T>
inline void deserializeJS(QVector<T> & cl, const quint8 * data, int len, JSDeserializerBase &)
{
	JSDeserializerBase ser(data, len);
	cl.clear();
	while (ser.isAnyAvailable()) {
		T el; ser >> el;
		cl.append(el);
	}
}

// ----------------------------------------------------------------------------
// QSet
// ----------------------------------------------------------------------------

template <typename T>
inline void serializeJS(const QSet<T> & cl, QByteArray & data, JSSerializerBase &)
{
	JSSerializerBase ser(data);
	for (typename QSet<T>::const_iterator it = cl.constBegin() ; it != cl.constEnd() ; ++it) ser << *it;
}

template <typename T>
inline void deserializeJS(QSet<T> & cl, const quint8 * data, int len, JSDeserializerBase &)
{
	JSDeserializerBase ser(data, len);
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
inline void serializeJS(const QHash<Key, T> & cl, QByteArray & data, JSSerializerBase &)
{
	JSSerializerBase ser(data);
	typename QHash<Key, T>::const_iterator it = cl.end();
	typename QHash<Key, T>::const_iterator begin = cl.begin();
	while (it != begin) {
		--it;
		ser << it.key() << it.value();
	}
}

template <typename Key, typename T>
inline void deserializeJS(QHash<Key, T> & cl, const quint8 * data, int len, JSDeserializerBase &)
{
	JSDeserializerBase ser(data, len);
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
inline void serializeJS(const QMap<Key, T> & cl, QByteArray & data, JSSerializerBase &)
{
	JSSerializerBase ser(data);
	typename QMap<Key, T>::const_iterator it = cl.end();
	typename QMap<Key, T>::const_iterator begin = cl.begin();
	while (it != begin) {
		--it;
		ser << it.key() << it.value();
	}
}

template <typename Key, typename T>
inline void deserializeJS(QMap<Key, T> & cl, const quint8 * data, int len, JSDeserializerBase &)
{
	JSDeserializerBase ser(data, len);
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
inline void serializeJS(const T & cl, QByteArray & data, JSSerializerBase &)
{
	JSSerializerBase ser(data);
	cl.serialize(ser);
}

template <typename T>
inline void deserializeJS(T & cl, const quint8 * data, int len, JSDeserializerBase &)
{
	JSDeserializerBase ser(data, len);
	cl.deserialize(ser);
}


}}}	// namespace

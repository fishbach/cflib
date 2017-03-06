/* Copyright (C) 2013-2017 Christian Fischbach <cf@cflib.de>
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

#include <QtCore>

namespace cflib { namespace util { namespace log {

namespace {

template<typename T>
inline void writeUInt(QByteArray & dest, T val)
{
	// most common case
	if (val == 0) { dest += '0'; return; }

	// calc size
	int size = dest.size();
	for (T i = val ; i > 0 ; i /= 10) ++size;

	// write int
	dest.resize(size);
	char * pos = (char *)dest.constData() + size;	// constData for performance
	for ( ; val > 0 ; val /= 10) *(--pos) = '0' + (val % 10);
}

template<typename T>
inline void writeInt(QByteArray & dest, T val)
{
	// most common case
	if (val == 0) { dest += '0'; return; }

	int size = dest.size();

	// check negation
	const bool neg = val < 0;
	if (neg) {
		val *= -1;
		++size;
	}

	// calc size
	for (T i = val ; i > 0 ; i /= 10) ++size;

	// write int
	dest.resize(size);
	char * pos = (char *)dest.constData() + size;	// constData for performance
	for ( ; val > 0 ; val /= 10) *(--pos) = '0' + (val % 10);
	if (neg) *(--pos) = '-';
}

}

// integer
inline void logFormat(QByteArray & dest, bool    val) { dest.append(val ? "true" : "false"); }
inline void logFormat(QByteArray & dest, qint8   val) { writeInt (dest, val); }
inline void logFormat(QByteArray & dest, quint8  val) { writeUInt(dest, val); }
inline void logFormat(QByteArray & dest, qint16  val) { writeInt (dest, val); }
inline void logFormat(QByteArray & dest, quint16 val) { writeUInt(dest, val); }
inline void logFormat(QByteArray & dest, qint32  val) { writeInt (dest, val); }
inline void logFormat(QByteArray & dest, quint32 val) { writeUInt(dest, val); }
inline void logFormat(QByteArray & dest, qint64  val) { writeInt (dest, val); }
inline void logFormat(QByteArray & dest, quint64 val) { writeUInt(dest, val); }
inline void logFormat(QByteArray & dest, void *  ptr) { writeUInt(dest, (quintptr)ptr); }

// floating point
inline void logFormat(QByteArray & dest, float  val) { dest += QByteArray::number(val); }
inline void logFormat(QByteArray & dest, double val) { dest += QByteArray::number(val); }

// strings
inline void logFormat(QByteArray & dest, char * str)            { dest += str; }
inline void logFormat(QByteArray & dest, const char * str)      { dest += str; }
inline void logFormat(QByteArray & dest, const QByteArray & ba) { dest += ba; }
inline void logFormat(QByteArray & dest, const QChar & c)       { dest += QString(c).toUtf8(); }
inline void logFormat(QByteArray & dest, const QString & str)   { dest += str.toUtf8(); }

// Qt classes
inline void logFormat(QByteArray & dest, const QTime     & ti) { dest += ti.toString().toUtf8(); }
inline void logFormat(QByteArray & dest, const QDateTime & dt) {
	dest += dt.toString("dd.MM.yyyy HH:mm:ss.zzz").toUtf8();
	if (dt.timeSpec() == Qt::UTC) dest += " UTC";
	else                          dest += " Local";
}

template<typename T>
inline void logFormat(QByteArray & dest, const T & val) { dest += val.toString().toUtf8(); }

}}}	// namespace

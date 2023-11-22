/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/util/timer.h>

#include <QtCore>

#define directConnect(o1, f1, o2, f2) QObject::connect(o1, f1, o2, f2, Qt::DirectConnection)

#define MultiLineStr(...) #__VA_ARGS__

inline QByteArray operator<<(const char * lhs, const QByteArray & rhs) { QByteArray ba(lhs); return ba += rhs; }
inline QByteArray & operator<<(QByteArray & lhs, const QByteArray & rhs) { return lhs += rhs; }
inline QByteArray & operator<<(QByteArray & lhs, const char * rhs) { return lhs += rhs; }
inline QByteArray & operator<<(QByteArray & lhs, char rhs) { return lhs += rhs; }

inline QString & operator<<(QString & lhs, const QString & rhs) { return lhs += rhs; }
inline QString & operator<<(QString & lhs, const char * rhs) { return lhs += rhs; }
inline QString & operator<<(QString & lhs, char rhs) { return lhs += rhs; }

namespace cflib { namespace util {

QByteArray weekDay(const QDate & date);
QByteArray dateTimeForHTTP(const QDateTime & dateTime);

quint32 calcCRC32Raw(quint32 crc, const char * data, quint64 size);
inline quint32 calcCRC32(const char * data, quint64 size) { return calcCRC32Raw(0xffffffffL, data, size) ^ 0xffffffffL; }
inline quint32 calcCRC32(const QByteArray & data) { return calcCRC32(data.constData(), data.size()); }

// 0 -> no compression, 1 -> fast, 9 -> small
void gzip(QByteArray & data, int compressionLevel = -1);
void deflateRaw(QByteArray & data, int compressionLevel = -1);
void inflateRaw(QByteArray & data);

QByteArray readFile(const QString & path);
bool writeFile(const QString & path, const QByteArray & data, QFile::Permissions perm =
	QFile::ReadOwner  | QFile::ReadUser | QFile::ReadGroup | QFile::ReadOther |
	QFile::WriteOwner | QFile::WriteUser);
QString readTextfile(const QString & path);

inline QChar decomposedBase(const QChar & chr)
{
	if (chr.decompositionTag() != QChar::NoDecomposition) return chr.decomposition()[0];
	return chr;
}

inline QString decomposedBase(const QString & str)
{
	QString retval = str;
	for (QString::Iterator it = retval.begin() ; it != retval.end() ; ++it) {
		QChar & chr = *it;
		if (chr.decompositionTag() != QChar::NoDecomposition) chr = chr.decomposition()[0];
	}
	return retval;
}

QByteArray encodeQuotedPrintable(const QString & text);
QByteArray encodeWord(const QString & str, bool strict);

template<typename K, typename V>
QString intHashToString(const QHash<K, V> & hash)
{
	QString rv('[');
	QHashIterator<K, V> it(hash);
	bool isFirst = true;
	while (it.hasNext()) {
		it.next();
		if (isFirst) isFirst = false;
		else rv += ',';
		rv += QString::number(it.key());
		rv += ':';
		rv += it.value().toString();
	}
	rv += ']';
	return rv;
}

template<typename K, typename V>
QString intMapToString(const QMap<K, V> & hash)
{
	QString rv('[');
	QMapIterator<K, V> it(hash);
	bool isFirst = true;
	while (it.hasNext()) {
		it.next();
		if (isFirst) isFirst = false;
		else rv += ',';
		rv += QString::number(it.key());
		rv += ':';
		rv += it.value().toString();
	}
	rv += ']';
	return rv;
}

QString flatten(const QString & str);

inline QString & add(QStringList & list) { list << QString(); return list.last(); }

inline QDateTime readUTCDateTime(const QString & str)
{
	QDateTime rv = QDateTime::fromString(str, "yyyy-MM-dd HH:mm:ss");
	rv.setTimeSpec(Qt::UTC);
	return rv;
}

bool validWebInputChars(const QString & str);
bool isValidEmail(const QString & str);

bool daemonize();
bool setProcessOwner(int uid, int gid);
bool processRestarter(uint msDelay = 1000);

template<typename C> void deleteNext(const C * obj) { Timer::singleShot(0, new Deleter<C>(obj)); }

// currently only needed for OSX when using QGuiApplication ("App Nap")
// also needed in pro file:
// macx: LIBS += -framework Cocoa
void preventApplicationSuspend();

class LogProcStatus : public QObject
{
	Q_OBJECT
public:
	LogProcStatus(uint intervalMsec);
private slots:
	void timeout();
};

void threadSafeExit(int returnCode);

template <typename T>
class qAsConstKeyValue
{
public:
	qAsConstKeyValue(const T & container_) : container_(container_) {}
	auto begin() { return container_.constKeyValueBegin(); }
	auto end() { return container_.constKeyValueEnd(); }
private:
	const T & container_;
};

}}	// namespace

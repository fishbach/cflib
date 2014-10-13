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

#include "log.h"

#include <cflib/util/hex.h>

// needed for threadId()
#ifndef Q_OS_WIN
	#include <unistd.h>
	#include <sys/syscall.h>
	#ifdef _syscall0
		_syscall0(pid_t, gettid)
		pid_t gettid(void);
	#else
		inline pid_t gettid(void) {
			#ifdef Q_OS_MAC
				return (pid_t)syscall(SYS_thread_selfid);
			#else
				return (pid_t)syscall(__NR_gettid);
			#endif
		}
	#endif
#endif

namespace cflib { namespace util {

namespace {

bool active = false;
QMutex mutex;
QFile file;
LogCategory logLevelTrigger = 15;
LogLevelCallback logLevelCallback = 0;

struct ThreadInfo {
	uint indent;
	uint threadId;
	ThreadInfo() : indent(0), threadId(0) {}
};
QHash<uint, ThreadInfo> threadInfos;

inline uint threadId()
{
#ifdef Q_OS_WIN
	return (uint)GetCurrentThreadId();
#else
	return (uint)gettid();
#endif
}

inline void writeInt(char * dest, uint number, int width)
{
	dest += width;
	for (int i = 0 ; i < width ; ++i) {
		if (number == 0) {
			*(--dest) = '0';
		} else {
			*(--dest) = '0' + number % 10;
			number /= 10;
		}
	}
}

inline void writeIntPadded(char * dest, uint number, int width)
{
	dest += width;
	for (int i = 0 ; i < width ; ++i) {
		if (number == 0 && i > 0) {
			*(--dest) = ' ';
		} else {
			*(--dest) = '0' + number % 10;
			number /= 10;
		}
	}
}

inline void writeCategory(char * dest, LogCategory cat)
{
	static const char * LevelChar = "-FTDIWC";

	*(dest++) = toHex(cat >> 12);
	*(dest++) = toHex(cat >>  8 & 0xF);
	*(dest++) = toHex(cat >>  4 & 0xF);
	const uint lc = cat & 0xF;
	*(dest++) = LevelChar[lc > 6 ? 0 : lc];
}

// allow only ASCII for security reasons
inline void writeMsg(QByteArray & out, const QByteArray & msg)
{
	const char * start = msg.constData();
	const char * p = start;
	for (int i = 0 ; i < msg.length() ; ++i) {
		const quint8 c = (quint8)*p;
		if (c < 0x20 || c > 0x7E) {
			if (p > start) out.append(start, p - start);
			++p; start = p;

			out += '<';
			out += toHex(c >> 4);
			out += toHex(c & 0xF);
			out += '>';
		} else ++p;
	}
	if (p > start) out.append(start, p - start);
}

}

void Log::start(const QString & fileName)
{
	if (active) {
		QTextStream(stderr) << "logging already started with log file: " << file.fileName() << endl;
		return;
	}

	file.setFileName(fileName);
	if (!file.open(QFile::WriteOnly | QFile::Append)) {
		QTextStream(stderr) << "could not open log file: " << fileName  << " (" << file.errorString() << ")" << endl;
		return;
	}
	file.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup);
	active = true;

	writeLog(__FILE__, __LINE__, LogCat::Info, "started ...", 0);

	qInstallMessageHandler(&Log::qtMessageHandler);
}

void Log::setLevelCallback(LogCategory level, LogLevelCallback callback)
{
	logLevelTrigger = level;
	logLevelCallback = callback;
}

void Log::writeLog(const char * filename, int lineNo, LogCategory category, const QByteArray & msg,
	int indent)
{
	if (!active) return;

	// construct message
	QByteArray line;
	line.reserve(256);
	line.resize(54);
	char * pos = (char *)line.constData();	// constData for performance

	// timestamp
	const QDateTime now = QDateTime::currentDateTime();
	writeInt(pos, now.date().year(),  4); pos += 4;
	writeInt(pos, now.date().month(), 2); pos += 2;
	writeInt(pos, now.date().day(),   2); pos += 2;
	*(pos++) = '-';
	writeInt(pos, now.time().hour(),   2); pos += 2;
	writeInt(pos, now.time().minute(), 2); pos += 2;
	writeInt(pos, now.time().second(), 2); pos += 2;
	*(pos++) = '.';
	writeInt(pos, now.time().msec(),   3); pos += 3;
	*(pos++) = ' ';

	// category
	writeCategory(pos, category); pos += 4;
	*(pos++) = ' ';

	// filename
	{
		const char * f = filename + strlen(filename) - 20;
		for (int i = 0 ; i < 20 ; ++i) {
			if (f < filename) *pos = ' ';
			else              *pos = *f;
			++f; ++pos;
		}
	}

	// line no
	*(pos++) = ':';
	writeIntPadded(pos, lineNo, 4); pos += 4;
	*(pos++) = ' ';

	// get thread info
	QMutexLocker lock(&mutex);
	ThreadInfo & info = threadInfos[threadId()];
	if (info.threadId == 0) info.threadId = threadInfos.size();

	// thread id
	writeInt(pos, info.threadId, 2); pos += 2;
	*(pos++) = ' ';

	// indent for log function trace
	if (indent < 0) {
		info.indent += indent;
		if (info.indent > 0) line += QByteArray(info.indent, ' ');
		line += "}\n";
	} else {
		if (info.indent > 0) line += QByteArray(info.indent, ' ');
		if (indent > 0) {
			line += msg;
			line += " {\n";
			info.indent += indent;
		} else {
			// write message
			writeMsg(line, msg);
			line += '\n';
		}
	}

	// write to file
	file.write(line);
	file.flush();

	// log level callback
	if (logLevelCallback && (category & 0x0F) >= logLevelTrigger) logLevelCallback(line);
}

void Log::qtMessageHandler(QtMsgType type, const QMessageLogContext & context, const QString & msg)
{
	LogCategory cat = 0;
	switch (type) {
		case QtDebugMsg:    cat = LogCat::Debug; break;
		case QtWarningMsg:  cat = LogCat::Warn;  break;
		case QtCriticalMsg:
		case QtFatalMsg:    cat = LogCat::Critical; break;
	}
	Log::writeLog(context.file, context.line, cat, msg.toLatin1(), 0);
	if (type == QtFatalMsg) ::abort();
}

}}	// namespace

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

#include <QtCore>

#include <cflib/util/impl/logformat.h>

#define USE_LOG(category) \
	namespace { const cflib::util::LogFileInfo cflib_util_logFileInfo(__FILE__, (category)); }
#define USE_LOG_MEMBER(category) \
	const cflib::util::LogFileInfo cflib_util_logFileInfo = cflib::util::LogFileInfo(__FILE__, (category));

#define logTrace    cflib::util::Log(cflib_util_logFileInfo, __LINE__, LogCat::Trace)
#define logDebug    cflib::util::Log(cflib_util_logFileInfo, __LINE__, LogCat::Debug)
#define logInfo     cflib::util::Log(cflib_util_logFileInfo, __LINE__, LogCat::Info)
#define logWarn     cflib::util::Log(cflib_util_logFileInfo, __LINE__, LogCat::Warn)
#define logCritical cflib::util::Log(cflib_util_logFileInfo, __LINE__, LogCat::Critical)

#define logCustom(category) cflib::util::Log(cflib_util_logFileInfo, __LINE__, (category))

#define logFunctionTrace \
	const cflib::util::LogFunctionTrace cflib_util_logFunctionTrace(cflib_util_logFileInfo, __LINE__, Q_FUNC_INFO);

#define logFunctionTraceParam \
	const cflib::util::LogFunctionTrace cflib_util_logFunctionTrace(cflib_util_logFileInfo, __LINE__); \
	cflib_util_logFunctionTrace

namespace LogCat { enum {
	None = 0,

	FuncTrace = 1,
	Trace     = 2,
	Debug     = 3,
	Info      = 4,
	Warn      = 5,
	Critical  = 6,

	Etc     = 1 <<  4,
	Network = 1 <<  5,
	Db      = 1 <<  6,
	Http    = 1 <<  7,
	Crypt   = 1 <<  8,
	User    = 1 <<  9,
	JS      = 1 << 10,
	Compute = 1 << 11
}; }

namespace cflib { namespace util {

typedef quint16 LogCategory;
typedef void (*LogLevelCallback)(const QByteArray & msg);

struct LogFileInfo
{
	LogFileInfo(const char * file, LogCategory category) : file(file), category(category) {}
	const char * const file;
	const LogCategory category;
};

class Log
{
public:
	static void start(const QString & fileName);
	static void setLevelCallback(LogCategory level, LogLevelCallback callback);

public:
	Log(LogFileInfo fi, int line, LogCategory category)
	: fi_(fi), line_(line), category_(category) {}

	inline void operator()(const char * str) const {
		if (check()) writeLog(str);
	}

	inline void operator()(const QByteArray & str) const {
		if (check()) writeLog(str);
	}

	template<typename T1>
	inline void operator()(const char * str, const T1 & t1) const {
		if (!check()) return;
		QByteArray msg; msg.reserve(128);
		const char * p = str;
		while (*p) {
			if (*p == '%' && *(p+1) == '1') {
				msg.append(str, p - str);
				log::logFormat(msg, t1);
				p += 2; str = p;
					continue;
			}
			++p;
		}
		if (str != p) msg.append(str, p - str);
		writeLog(msg);
	}

	template<typename T1, typename T2>
	void operator()(const char * str, const T1 &  t1, const T2 & t2) const {
		if (!check()) return;
		QByteArray msg; msg.reserve(128);
		const char * p = str;
		while (*p) {
			if (*p == '%') {
				const char c = *(p+1);
				if (c >= '1' && c <= '2') {
					msg.append(str, p - str);
					if (c == '1') log::logFormat(msg, t1);
					else          log::logFormat(msg, t2);
					p += 2; str = p;
					continue;
				}
			}
			++p;
		}
		if (str != p) msg.append(str, p - str);
		writeLog(msg);
	}

	template<typename T1, typename T2, typename T3>
	void operator()(const char * str, const T1 & t1, const T2 & t2, const T3 & t3) const {
		if (!check()) return;
		QByteArray msg; msg.reserve(128);
		const char * p = str;
		while (*p) {
			if (*p == '%') {
				const char c = *(p+1);
				if (c >= '1' && c <= '3') {
					msg.append(str, p - str);
					switch (c) {
						case '1': log::logFormat(msg, t1); break;
						case '2': log::logFormat(msg, t2); break;
						default : log::logFormat(msg, t3); break;
					}
					p += 2; str = p;
					continue;
				}
			}
			++p;
		}
		if (str != p) msg.append(str, p - str);
		writeLog(msg);
	}

	template<typename T1, typename T2, typename T3, typename T4>
	void operator()(const char * str, const T1 & t1, const T2 & t2, const T3 & t3, const T4 & t4) const {
		if (!check()) return;
		QByteArray msg; msg.reserve(128);
		const char * p = str;
		while (*p) {
			if (*p == '%') {
				const char c = *(p+1);
				if (c >= '1' && c <= '4') {
					msg.append(str, p - str);
					switch (c) {
						case '1': log::logFormat(msg, t1); break;
						case '2': log::logFormat(msg, t2); break;
						case '3': log::logFormat(msg, t3); break;
						default : log::logFormat(msg, t4); break;
					}
					p += 2; str = p;
					continue;
				}
			}
			++p;
		}
		if (str != p) msg.append(str, p - str);
		writeLog(msg);
	}

	template<typename T1, typename T2, typename T3, typename T4, typename T5>
	void operator()(const char * str, const T1 & t1, const T2 & t2, const T3 & t3, const T4 & t4, const T5 & t5) const {
		if (!check()) return;
		QByteArray msg; msg.reserve(128);
		const char * p = str;
		while (*p) {
			if (*p == '%') {
				const char c = *(p+1);
				if (c >= '1' && c <= '5') {
					msg.append(str, p - str);
					switch (c) {
						case '1': log::logFormat(msg, t1); break;
						case '2': log::logFormat(msg, t2); break;
						case '3': log::logFormat(msg, t3); break;
						case '4': log::logFormat(msg, t4); break;
						default : log::logFormat(msg, t5); break;
					}
					p += 2; str = p;
					continue;
				}
			}
			++p;
		}
		if (str != p) msg.append(str, p - str);
		writeLog(msg);
	}

	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	void operator()(const char * str, const T1 & t1, const T2 & t2, const T3 & t3, const T4 & t4, const T5 & t5,
		const T6 & t6) const
	{
		if (!check()) return;
		QByteArray msg; msg.reserve(128);
		const char * p = str;
		while (*p) {
			if (*p == '%') {
				const char c = *(p+1);
				if (c >= '1' && c <= '6') {
					msg.append(str, p - str);
					switch (c) {
						case '1': log::logFormat(msg, t1); break;
						case '2': log::logFormat(msg, t2); break;
						case '3': log::logFormat(msg, t3); break;
						case '4': log::logFormat(msg, t4); break;
						case '5': log::logFormat(msg, t5); break;
						default : log::logFormat(msg, t6); break;
					}
					p += 2; str = p;
					continue;
				}
			}
			++p;
		}
		if (str != p) msg.append(str, p - str);
		writeLog(msg);
	}

	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	void operator()(const char * str, const T1 & t1, const T2 & t2, const T3 & t3, const T4 & t4, const T5 & t5,
		const T6 & t6, const T7 & t7) const
	{
		if (!check()) return;
		QByteArray msg; msg.reserve(128);
		const char * p = str;
		while (*p) {
			if (*p == '%') {
				const char c = *(p+1);
				if (c >= '1' && c <= '7') {
					msg.append(str, p - str);
					switch (c) {
						case '1': log::logFormat(msg, t1); break;
						case '2': log::logFormat(msg, t2); break;
						case '3': log::logFormat(msg, t3); break;
						case '4': log::logFormat(msg, t4); break;
						case '5': log::logFormat(msg, t5); break;
						case '6': log::logFormat(msg, t6); break;
						default : log::logFormat(msg, t7); break;
					}
					p += 2; str = p;
					continue;
				}
			}
			++p;
		}
		if (str != p) msg.append(str, p - str);
		writeLog(msg);
	}

private:
	inline bool check() const {
		return true;
	}

	virtual void writeLog(const QByteArray & msg) const {
		writeLog(fi_.file, line_, fi_.category | category_, msg, 0);
	}

	static void writeLog(const char * filename, int lineNo, LogCategory category, const QByteArray & msg,
		int indent);

	static void qtMessageHandler(QtMsgType type, const QMessageLogContext & context, const QString & msg);

private:
	const LogFileInfo fi_;
	const int line_;
	const LogCategory category_;
	friend class LogFunctionTrace;
};

class LogFunctionTrace : public Log
{
public:
	LogFunctionTrace(LogFileInfo fi, int line) :
		Log(fi, line, LogCat::FuncTrace)
	{
	}

	LogFunctionTrace(LogFileInfo fi, int line, const char * funcName) :
		Log(fi, line, LogCat::FuncTrace)
	{
		writeLog(funcName);
	}

	~LogFunctionTrace() {
		Log::writeLog(fi_.file, line_, fi_.category | category_, QByteArray(), -2);
	}

private:
	virtual void writeLog(const QByteArray & msg) const {
		Log::writeLog(fi_.file, line_, fi_.category | category_, msg, 2);
	}
};

}}	// namespace

/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "log.h"

#include <cflib/base/cfconcurrent.h>
#include <cflib/base/cfdatetime.h>
#include <cflib/base/cffile.h>
#include <cflib/base/cfcontainers.h>
#include <cflib/util/hex.h>

// needed for threadId()
#include <unistd.h>
#ifdef __APPLE__
    #include <pthread.h>
#else
    #include <sys/syscall.h>
#endif
#ifdef _syscall0
    _syscall0(pid_t, gettid)
    pid_t gettid(void);
#else
    inline pid_t gettid(void) {
        #ifdef __APPLE__
            uint64_t tid;
            pthread_threadid_np(NULL, &tid);
            return (pid_t)tid;
        #else
            return (pid_t)syscall(__NR_gettid);
        #endif
    }
#endif

#include <cstdio>

namespace cflib { namespace util {

namespace {

bool active = false;
CFMutex mutex;
CFFile file;
LogCategory logLevelTrigger = 15;
LogLevelCallback logLevelCallback = 0;

struct ThreadInfo {
    cfuint indent;
    cfuint threadId;
    ThreadInfo() : indent(0), threadId(0) {}
};
CFHash<cfuint, ThreadInfo> threadInfos;

inline cfuint threadId()
{
    return (cfuint)gettid();
}

inline void writeInt(char * dest, cfuint number, int width)
{
    // bug in gcc
    #pragma GCC diagnostic push
    #if defined(__has_warning)
        #if __has_warning("-Wstringop-overflow=")
            #pragma GCC diagnostic ignored "-Wstringop-overflow="
        #endif
    #endif

    dest += width;
    for (int i = 0 ; i < width ; ++i) {
        if (number == 0) {
            *(--dest) = '0';
        } else {
            *(--dest) = '0' + number % 10;
            number /= 10;
        }
    }

    #pragma GCC diagnostic pop
}

inline void writeIntPadded(char * dest, cfuint number, int width)
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
    const cfuint lc = cat & 0xF;
    *(dest++) = LevelChar[lc > 6 ? 0 : lc];
}

// allow only ASCII for security reasons
inline void writeMsg(CFByteArray & out, const CFByteArray & msg)
{
    const char * start = msg.constData();
    const char * p = start;
    for (cfsize_t i = 0 ; i < msg.length() ; ++i) {
        const cfuint8 c = (cfuint8)*p;
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

LogCategory Log::logLevelCategory_ = 0;

void Log::start(const CFString & fileName)
{
    if (active) {
        fprintf(stderr, "logging already started with log file: %s\n", file.fileName().c_str());
        return;
    }

    if (fileName == "-") {
        file.open(1, CFFile::WriteOnly);
    } else {
        file.setFileName(fileName);
        if (!file.open(CFFile::WriteOnly | CFFile::Append)) {
            fprintf(stderr, "could not open log file: %s (%s)\n", fileName.c_str(), file.errorString().c_str());
            return;
        }
        file.setPermissions(CFFile::ReadOwner | CFFile::WriteOwner | CFFile::ReadGroup);
    }
    active = true;
}

void Log::setLevelCallback(LogCategory level, LogLevelCallback callback)
{
    logLevelTrigger = level;
    logLevelCallback = callback;
}

void Log::writeLog(const char * filename, int lineNo, LogCategory category, const CFByteArray & msg,
    int indent)
{
    if (!active) return;

    // construct message
    CFByteArray line;
    line.reserve(256);
    line.resize(54);
    char * pos = (char *)line.constData();    // constData for performance

    // bug in gcc
    #pragma GCC diagnostic push
    #if defined(__has_warning)
        #if __has_warning("-Wstringop-overflow=")
            #pragma GCC diagnostic ignored "-Wstringop-overflow="
        #endif
    #endif

    // timestamp
    const CFDateTime now = CFDateTime::nowUTC();
    writeInt(pos, now.year(),  4); pos += 4;
    writeInt(pos, now.month(), 2); pos += 2;
    writeInt(pos, now.day(),   2); pos += 2;
    *(pos++) = '-';
    writeInt(pos, now.hour(),   2); pos += 2;
    writeInt(pos, now.minute(), 2); pos += 2;
    writeInt(pos, now.second(), 2); pos += 2;
    *(pos++) = '.';
    writeInt(pos, now.msec(),   3); pos += 3;
    *(pos++) = ' ';

    // category
    writeCategory(pos, category); pos += 4;
    *(pos++) = ' ';

    // filename
    if (filename) {
        const char * f = filename + strlen(filename) - 20;
        for (int i = 0 ; i < 20 ; ++i) {
            if (f < filename) *pos = ' ';
            else              *pos = *f;
            ++f; ++pos;
        }
    } else {
        for (int i = 0 ; i < 20 ; ++i) *(pos++) = ' ';
    }

    // line no
    *(pos++) = ':';
    writeIntPadded(pos, lineNo, 4); pos += 4;
    *(pos++) = ' ';

    // get thread info
    {
        CFMutexLocker lock(mutex);
        ThreadInfo & info = threadInfos[threadId()];
        if (info.threadId == 0) info.threadId = (cfuint)threadInfos.size();

        // thread id
        writeInt(pos, info.threadId, 2); pos += 2;
        *(pos++) = ' ';

        #pragma GCC diagnostic pop

        // indent for log function trace
        if (indent < 0) {
            info.indent += indent;
            if (info.indent > 0) line += CFByteArray((cfsize_t)info.indent, ' ');
            line += "}\n";
        } else {
            if (info.indent > 0) line += CFByteArray((cfsize_t)info.indent, ' ');
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
    }

    // log level callback
    if (logLevelCallback && (category & 0x0F) >= logLevelTrigger) logLevelCallback(line);
}

}}    // namespace

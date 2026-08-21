/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "log.h"

#include <cflib/base.h>
#include <cflib/util/hex.h>
#include <cflib/util/thread.h>

#include <format>
#include <iostream>

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

namespace cflib::util {

namespace {

bool active = false;
Mutex mutex;
File file;
LogCategory logLevelTrigger = 15;
LogLevelCallback logLevelCallback = 0;

struct ThreadInfo {
    uint indent = 0;
};
Hash<uint, ThreadInfo> threadInfos;

inline size_t getThreadId()
{
    size_t id = Thread::currentId();
    if (id > 0) return id;
    return (size_t)gettid();
}

inline void writeInt(char * dest, uint number, int width)
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
inline void writeMsg(ByteArray & out, const ByteArray & msg)
{
    const char * start = msg.constCharPtr();
    const char * p = start;
    for (size_t i = 0 ; i < msg.length() ; ++i) {
        const uint8 c = (uint8)*p;
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

void Log::start(const String & fileName)
{
    if (active) {
        std::cerr << std::format("logging already started with log file: {}\n", file.fileName().toStdString());
        return;
    }

    if (fileName == "-") {
        file.open(1, File::WriteOnly);
    } else {
        file.setFileName(fileName);
        if (!file.open(File::WriteOnly | File::Append)) {
            std::cerr << std::format("could not open log file: {} ({})\n", fileName.toStdString(), file.errorString().toStdString());
            return;
        }
        file.setPermissions(File::ReadOwner | File::WriteOwner | File::ReadGroup);
    }
    active = true;
}

void Log::setLevelCallback(LogCategory level, LogLevelCallback callback)
{
    logLevelTrigger = level;
    logLevelCallback = callback;
}

void Log::writeLog(const char * filename, int lineNo, LogCategory category, const ByteArray & msg,
    int indent)
{
    if (!active) return;

    // construct message
    ByteArray line;
    line.reserve(256);
    line.resize(54);
    char * pos = line.charPtr();

    // bug in gcc
    #pragma GCC diagnostic push
    #if defined(__has_warning)
        #if __has_warning("-Wstringop-overflow=")
            #pragma GCC diagnostic ignored "-Wstringop-overflow="
        #endif
    #endif

    // timestamp
    const DateTime now = DateTime::nowUTC();
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
        MutexLocker lock(mutex);
        size_t threadId = getThreadId();
        ThreadInfo & info = threadInfos[threadId];

        // thread id
        writeInt(pos, threadId, 2); pos += 2;
        *(pos++) = ' ';

        #pragma GCC diagnostic pop

        // indent for log function trace
        if (indent < 0) {
            info.indent += indent;
            if (info.indent > 0) line += ByteArray((size_t)info.indent, ' ');
            line += "}\n";
        } else {
            if (info.indent > 0) line += ByteArray((size_t)info.indent, ' ');
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

} // namespace

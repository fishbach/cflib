/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>
#include <cflib/util/timer.h>

#define MultiLineStr(...) #__VA_ARGS__

namespace cflib::util {

ByteArray weekDay(int dayOfWeek);
ByteArray dateTimeForHTTP(const CFDateTime & dateTime);

cfuint32 calcCRC32Raw(cfuint32 crc, const char * data, cfuint64 size);
inline cfuint32 calcCRC32(const char * data, cfuint64 size) { return calcCRC32Raw(0xffffffffL, data, size) ^ 0xffffffffL; }
inline cfuint32 calcCRC32(const ByteArray & data) { return calcCRC32(data.constData(), data.size()); }

// 0 -> no compression, 1 -> fast, 9 -> small
void gzip(ByteArray & data, int compressionLevel = -1);
void deflateRaw(ByteArray & data, int compressionLevel = -1);
void inflateRaw(ByteArray & data);

ByteArray readFile(const String & path);
bool writeFile(const String & path, const ByteArray & data, int perm =
    CFFile::ReadOwner  | CFFile::ReadUser | CFFile::ReadGroup | CFFile::ReadOther |
    CFFile::WriteOwner | CFFile::WriteUser);
String readTextfile(const String & path);

ByteArray encodeQuotedPrintable(const String & text);
ByteArray encodeWord(const String & str, bool strict);

String flatten(const String & str);

bool validWebInputChars(const String & str);
bool isValidEmail(const String & str);

bool daemonize();
bool setProcessOwner(int uid, int gid);
bool processRestarter(cfuint msDelay = 1000);

template<typename C> void deleteNext(const C * obj) { Timer::singleShot(0, new Deleter<C>(obj)); }

// currently only needed for OSX when using GUI ("App Nap")
// also needed in build: -framework Cocoa
void preventApplicationSuspend();

void threadSafeExit(int returnCode);

bool mkPath(const String & path);
bool removeFile(const String & path);
bool copyFile(const String & src, const String & dest);

} // namespace

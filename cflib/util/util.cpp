/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "util.h"

#include <cflib/util/log.h>

#include <zlib.h>

#include <cerrno>
#include <format>
#include <csignal>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

USE_LOG(LogCat::Etc)

namespace cflib::util {

ByteArray weekDay(int dayOfWeek)
{
    switch (dayOfWeek) {
        case  1: return "Mon";
        case  2: return "Tue";
        case  3: return "Wed";
        case  4: return "Thu";
        case  5: return "Fri";
        case  6: return "Sat";
        case  7: return "Sun";
        default: return "";
    }
}

ByteArray dateTimeForHTTP(const DateTime & dateTime)
{
    // see RFC 2822 section 3.3.
    ByteArray retval = weekDay(dateTime.dayOfWeek());
    retval += ", ";

    retval += std::format("{:02d} ___ {:04d} {:02d}:{:02d}:{:02d}",
        dateTime.day(), dateTime.year(),
        dateTime.hour(), dateTime.minute(), dateTime.second()).c_str();

    static const char * months[] = {
        "", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    int m = dateTime.month();
    if (m >= 1 && m <= 12) retval.replace("___", months[m]);
    else retval.replace("___", "");

    retval += " GMT";
    return retval;
}

namespace {

// CRC table - little endian
const uint32 CRCData[] = {
    0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
    0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
    0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
    0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
    0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
    0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
    0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
    0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
    0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
    0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
    0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
    0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
    0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
    0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
    0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
    0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
    0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
    0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
    0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
    0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
    0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
    0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
    0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
    0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
    0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
    0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
    0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
    0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
    0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
    0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
    0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
    0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
    0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
    0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
    0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
    0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
    0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
    0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
    0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
    0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
    0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
    0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
    0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
    0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
    0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
    0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
    0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
    0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
    0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
    0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
    0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
    0x2d02ef8dL
};

}

uint32 calcCRC32Raw(uint32 crc, const char * data, uint64 size)
{
    const uint8 * bytes = (const uint8 *)data;
    while (size--) {
        crc = CRCData[(crc & 0xff) ^ *(bytes++)] ^ (crc >> 8);
    }
    return crc;
}

void gzip(ByteArray & data, int compressionLevel)
{
    if (data.isEmpty()) {
        data = ByteArray::fromHex("1f8b08000000000000ff03000000000000000000");
        return;
    }

    const uint32 len = data.size();
    const uint32 crc = calcCRC32(data);

    // Use zlib deflate with gzip wrapper (windowBits = 31 = 15 + 16)
    z_stream s;
    s.zalloc    = Z_NULL;
    s.zfree     = Z_NULL;
    s.opaque    = Z_NULL;
    s.avail_in  = (uInt)data.size();
    s.next_in   = (Bytef *)data.constData();
    deflateInit2(&s, compressionLevel, Z_DEFLATED, 31, 8, Z_DEFAULT_STRATEGY);
    ByteArray out((size_t)deflateBound(&s, data.size()), '\0');
    s.avail_out = (uInt)out.size();
    s.next_out  = (Bytef *)out.data();
    deflate(&s, Z_FINISH);
    out.resize(s.total_out);
    deflateEnd(&s);

    // zlib with windowBits=31 produces a full gzip stream, but we need to fix the
    // OS field and potentially other fields to match the old qCompress-based output.
    // The simplest approach: keep zlib's output but patch the header to match.
    // Actually, let's just use the zlib gzip output directly.
    // But to match the old behavior exactly (with the specific header bytes), we need
    // to produce the same format. The old code used qCompress (zlib deflate wrapper)
    // then manually built gzip headers. Let's use the raw deflate approach instead.

    // Reset - use raw deflate, then manually build gzip envelope (matching old behavior)
    z_stream s2;
    s2.zalloc    = Z_NULL;
    s2.zfree     = Z_NULL;
    s2.opaque    = Z_NULL;
    s2.avail_in  = (uInt)data.size();
    s2.next_in   = (Bytef *)data.constData();
    deflateInit2(&s2, compressionLevel, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);
    ByteArray compressed((size_t)deflateBound(&s2, data.size()), '\0');
    s2.avail_out = (uInt)compressed.size();
    s2.next_out  = (Bytef *)compressed.data();
    deflate(&s2, Z_FINISH);
    compressed.resize(s2.total_out);
    deflateEnd(&s2);

    // Build gzip: header(10) + compressed + crc(4) + len(4)
    data.resize(0);
    data.reserve(10 + compressed.size() + 8);
    // gzip header
    data += '\x1f';
    data += '\x8b';
    data += '\x08';  // method: deflate
    data += '\x00';  // flags
    data += '\x00'; data += '\x00'; data += '\x00'; data += '\x00';  // mtime
    data += '\x00';  // xfl
    data += '\xff';  // OS: unknown
    // compressed data
    data += compressed;
    // CRC32 (little-endian)
    data += (char)(crc & 0xFF);
    data += (char)((crc >> 8) & 0xFF);
    data += (char)((crc >> 16) & 0xFF);
    data += (char)((crc >> 24) & 0xFF);
    // Original size (little-endian)
    data += (char)(len & 0xFF);
    data += (char)((len >> 8) & 0xFF);
    data += (char)((len >> 16) & 0xFF);
    data += (char)((len >> 24) & 0xFF);
}

void deflateRaw(ByteArray & data, int compressionLevel)
{
    if (data.isEmpty()) {
        data += '\0';
        return;
    }
    z_stream s;
    s.zalloc    = Z_NULL;
    s.zfree     = Z_NULL;
    s.opaque    = Z_NULL;
    s.avail_in  = (uInt)   data.size();
    s.next_in   = (Bytef *)data.constData();
    deflateInit2(&s, compressionLevel, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);
    ByteArray out((size_t)deflateBound(&s, data.size()), '\0');
    s.avail_out = (uInt)   out.size();
    s.next_out  = (Bytef *)out.constData();
    deflate(&s, Z_SYNC_FLUSH);
    deflateEnd(&s);
    int size = s.total_out;
    if (size > 0) {
        if (out[0] == '\0') --size;
        else if (size >= 4) size -= 4;
    }
    out.resize(size);
    data = out;
}

void inflateRaw(ByteArray & data)
{
    if (data.isEmpty()) return;
    const char fb = data[0];
    ByteArray in;
    in.reserve(data.size() + (fb == 0 ? 0 : 4));
    in.append(data);
    if (fb == 0) in.append("\x00\x00\xFF\xFF", 4);
    ByteArray out((size_t)5, '\0');
    z_stream s;
    s.zalloc    = Z_NULL;
    s.zfree     = Z_NULL;
    s.opaque    = Z_NULL;
    s.avail_in  = (uInt)   in.size();
    s.next_in   = (Bytef *)in.constData();
    s.avail_out = (uInt)   out.size();
    s.next_out  = (Bytef *)out.constData();
    inflateInit2(&s, -15);
    for (;;) {
        int rv = inflate(&s, Z_NO_FLUSH);
        if (rv != Z_OK && rv != Z_BUF_ERROR) logWarn("inflate error: %1", rv);
        if (s.avail_out > 0) break;
        const size_t oldSize = out.size();
        const size_t ext = oldSize * 3 / 2;
        out.resize(oldSize + ext);
        s.avail_out = (uInt)   ext;
        s.next_out  = (Bytef *)out.constData() + oldSize;
    }
    inflateEnd(&s);
    out.resize(s.total_out);
    data = out;
}

namespace {

const char * const Hex = "0123456789ABCDEF";

}

ByteArray encodeQuotedPrintable(const String & text)
{
    ByteArray utf8 = text.toUtf8();
    const unsigned char * pos = (const unsigned char *)utf8.constData();
    const unsigned char * second = pos + 1;
    const unsigned char * end = pos + utf8.length();
    ByteArray retval;
    int lineLen = 0;
    while (pos != end) {
        unsigned char c = *(pos++);
        if (c == 13 && pos != end && *pos == 10) {
            if (pos != second) {
                unsigned char w = *(pos - 2);
                if (w == 9 || w == 32) retval += "=\r\n";
            }
            retval += "\r\n";
            lineLen = 0;
            ++pos;
            continue;
        }

        if ((c >= 32 && c <= 60) || (c >= 62 && c <= 126) || c == 9) {
            if (lineLen == 75) {
                retval += "=\r\n";
                lineLen = 1;
            } else ++lineLen;
            retval += (char)c;
        } else {
            if (lineLen >= 73) {
                retval += "=\r\n";
                lineLen = 3;
            } else lineLen += 3;
            retval += '=';
            retval += Hex[c >> 4];
            retval += Hex[c & 15];
        }
    }
    return retval;
}

ByteArray encodeWord(const String & str, bool strict)
{
    ByteArray utf8 = str.toUtf8();
    const unsigned char * pos = (const unsigned char *)utf8.constData();
    const unsigned char * end = pos + utf8.length();
    ByteArray retval = "=?utf-8?Q?";
    bool onlyDirect = true;
    while (pos != end) {
        unsigned char c = *(pos++);
        if (c == 32) {
            retval += '_';
            continue;
        }
        bool direct = strict ? (
            (c >= 48 && c <= 57) || (c >= 65 && c <= 90) || (c >= 97 && c <= 122) ||
            c == 33 || c == 42 || c == 43 || c == 45 || c == 47
        ) : (
            (c >= 33 && c <= 60) || (c >= 64 && c <= 126) || c == 62
        );
        if (direct) {
            retval += (char)c;
        } else {
            retval += '=';
            retval += Hex[c >> 4];
            retval += Hex[c & 15];
            onlyDirect = false;
        }
    }
    if (onlyDirect) return str.toUtf8();
    retval += "?=";
    return retval;

}

String flatten(const String & str)
{
    // Hand-written replacement for QRegularExpression-based flatten
    // Step 1: remove non-allowed chars (keep a-zA-Z0-9 - . _ and whitespace)
    std::string rv;
    rv.reserve(str.size());
    for (size_t i = 0; i < str.size(); ++i) {
        char c = str.c_str()[i];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
            c == '-' || c == '.' || c == '_' || c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            rv += c;
        }
    }
    // Step 2: trim leading/trailing whitespace
    size_t start = 0;
    while (start < (size_t)rv.size() && (rv[start] == ' ' || rv[start] == '\t' || rv[start] == '\r' || rv[start] == '\n')) ++start;
    size_t end = (size_t)rv.size();
    while (end > start && (rv[end-1] == ' ' || rv[end-1] == '\t' || rv[end-1] == '\r' || rv[end-1] == '\n')) --end;
    rv = rv.substr(start, end - start);
    // Step 3: collapse whitespace to single underscore
    std::string result;
    result.reserve(rv.size());
    bool inSpace = false;
    for (char c : rv) {
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            if (!inSpace) {
                result += '_';
                inSpace = true;
            }
        } else {
            result += c;
            inSpace = false;
        }
    }
    // Step 4: collapse multiple underscores
    std::string final;
    final.reserve(result.size());
    bool lastUnderscore = false;
    for (char c : result) {
        if (c == '_') {
            if (!lastUnderscore) {
                final += '_';
                lastUnderscore = true;
            }
        } else {
            final += c;
            lastUnderscore = false;
        }
    }
    return String(std::move(final));
}

bool validWebInputChars(const String & str)
{
    const char * const NotAllowed = "{}[]<>;\"\\";
    const char * pos = NotAllowed;
    char c;
    while ((c = *(pos++))) if (str.contains(c)) return false;
    return true;
}

bool isValidEmail(const String & str)
{
    // Hand-written email validation replacing QRegularExpression
    // Pattern: ^[\w.\-_]+@\w[\w.\-]+\.\w+$
    const char * s = str.c_str();
    size_t len = str.size();
    if (len == 0) return false;

    size_t i = 0;
    // local part: [\w.\-_]+
    size_t localStart = i;
    while (i < len) {
        char c = s[i];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
            c == '_' || c == '.' || c == '-') {
            ++i;
        } else break;
    }
    if (i == localStart) return false;
    if (i >= len || s[i] != '@') return false;
    ++i; // skip @

    // domain: \w[\w.\-]+\.\w+
    if (i >= len) return false;
    char c = s[i];
    if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_'))
        return false;
    ++i;

    size_t lastDot = (size_t)-1;
    while (i < len) {
        c = s[i];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
            c == '_' || c == '.' || c == '-') {
            if (c == '.') lastDot = i;
            ++i;
        } else break;
    }
    if (i != len) return false;
    if (lastDot == (size_t)-1 || lastDot == len - 1) return false;
    // Check at least one char after the last dot that's a word char
    for (size_t j = lastDot + 1; j < len; ++j) {
        c = s[j];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_'))
            return false;
    }
    return true;
}

bool daemonize()
{
    pid_t pid = fork();
    if (pid < 0) return false;
    if (pid > 0) exit(0);
    if (setsid() < 0) return false;
    if (chdir("/") < 0) return false;
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    umask(0);
    return true;
}

bool setProcessOwner(int uid, int gid)
{
    return setgid(gid) != -1 && setuid(uid) != -1;
}

#ifndef __APPLE__
void preventApplicationSuspend()
{
}
#endif

namespace {

pid_t childPid = -1;
bool sigReceived = false;

void signalHandler(int sig)
{
    sigReceived = true;
    if (childPid > 0) kill(childPid, sig);
}

}

bool processRestarter(uint msDelay)
{
    for (;;) {
        childPid = fork();
        if (childPid < 0)  return false;
        if (childPid == 0) return true;

        sig_t oldSigH1  = ::signal(1,  signalHandler);
        sig_t oldSigH2  = ::signal(2,  signalHandler);
        sig_t oldSigH15 = ::signal(15, signalHandler);

        while (wait(NULL) != -1 && errno != ECHILD);
        if (sigReceived) exit(0);

        childPid = -1;
        usleep(msDelay * 2000);
        if (sigReceived) exit(0);

        ::signal(1,  oldSigH1);
        ::signal(2,  oldSigH2);
        ::signal(15, oldSigH15);
    }
}

void threadSafeExit(int returnCode)
{
    _exit(returnCode);
}

bool mkPath(const String & path)
{
    if (path.isEmpty()) return false;
    struct stat st;
    if (stat(path.c_str(), &st) == 0) return S_ISDIR(st.st_mode);

    // Recursively create parent
    size_t pos = path.str().rfind('/');
    if (pos != std::string::npos && pos > 0) {
        if (!mkPath(path.left(pos))) return false;
    }
    return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
}

bool removeFile(const String & path)
{
    return unlink(path.c_str()) == 0;
}

bool copyFile(const String & src, const String & dest)
{
    ByteArray data = File::read(src);
    return File::write(dest, data);
}

} // namespace

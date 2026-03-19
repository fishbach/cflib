/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>

#include <format>

namespace cflib::util::log {

namespace {

template<typename T>
inline void writeUInt(ByteArray & dest, T val)
{
    // most common case
    if (val == 0) { dest += '0'; return; }

    // calc size
    int size = dest.size();
    for (T i = val ; i > 0 ; i /= 10) ++size;

    // write int
    dest.resize(size);
    char * pos = (char *)dest.constData() + size;    // constData for performance
    for ( ; val > 0 ; val /= 10) *(--pos) = '0' + (val % 10);
}

template<typename T>
inline void writeInt(ByteArray & dest, T val)
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
    char * pos = (char *)dest.constData() + size;    // constData for performance
    for ( ; val > 0 ; val /= 10) *(--pos) = '0' + (val % 10);
    if (neg) *(--pos) = '-';
}

}

// integer
inline void logFormat(ByteArray & dest, bool    val) { dest.append(val ? "true" : "false"); }
inline void logFormat(ByteArray & dest, cfint8   val) { writeInt (dest, val); }
inline void logFormat(ByteArray & dest, cfuint8  val) { writeUInt(dest, val); }
inline void logFormat(ByteArray & dest, cfint16  val) { writeInt (dest, val); }
inline void logFormat(ByteArray & dest, cfuint16 val) { writeUInt(dest, val); }
inline void logFormat(ByteArray & dest, cfint32  val) { writeInt (dest, val); }
inline void logFormat(ByteArray & dest, cfuint32 val) { writeUInt(dest, val); }
inline void logFormat(ByteArray & dest, cfint64  val) { writeInt (dest, val); }
inline void logFormat(ByteArray & dest, cfuint64 val) { writeUInt(dest, val); }
#if defined(__APPLE__) || (defined(__linux__) && defined(__LP64__))
// On macOS and LP64 Linux, size_t is 'unsigned long' which is a distinct type from uint64_t (unsigned long long)
inline void logFormat(ByteArray & dest, unsigned long val) { writeUInt(dest, (cfuint64)val); }
inline void logFormat(ByteArray & dest, long val) { writeInt(dest, (cfint64)val); }
#endif
inline void logFormat(ByteArray & dest, void *  ptr) { writeUInt(dest, (cfuintptr)ptr); }

// floating point
inline void logFormat(ByteArray & dest, float  val) { dest += ByteArray::number(val); }
inline void logFormat(ByteArray & dest, double val) { dest += ByteArray::number(val); }

// strings
inline void logFormat(ByteArray & dest, char * str)            { dest += str; }
inline void logFormat(ByteArray & dest, const char * str)      { dest += str; }
inline void logFormat(ByteArray & dest, const ByteArray & ba) { dest += ba; }
inline void logFormat(ByteArray & dest, const String & str)   { dest.append(str.c_str()); }

// DateTime
inline void logFormat(ByteArray & dest, const DateTime & dt) {
    if (!dt.isValid()) { dest += "(null)"; return; }
    dest += std::format("{:02d}.{:02d}.{:04d} {:02d}:{:02d}:{:02d}.{:03d} UTC",
        dt.day(), dt.month(), dt.year(),
        dt.hour(), dt.minute(), dt.second(), dt.msec()).c_str();
}

template<typename T, typename = void>
struct HasToUtf8 : std::false_type {};
template<typename T>
struct HasToUtf8<T, std::void_t<decltype(std::declval<const T&>().toUtf8())>> : std::true_type {};

template<typename T, typename = void>
struct HasCharConstData : std::false_type {};
template<typename T>
struct HasCharConstData<T, std::enable_if_t<std::is_same_v<decltype(std::declval<const T&>().constData()), const char*>>> : std::true_type {};

template<typename T, typename = void>
struct HasToStringCStr : std::false_type {};
template<typename T>
struct HasToStringCStr<T, std::void_t<decltype(std::declval<const T&>().toString().c_str())>> : std::true_type {};

template<typename T, typename = void>
struct HasToStringToUtf8 : std::false_type {};
template<typename T>
struct HasToStringToUtf8<T, std::void_t<decltype(std::declval<const T&>().toString().toUtf8())>> : std::true_type {};

template<typename T>
inline auto logFormat(ByteArray & dest, const T & val)
    -> std::enable_if_t<!std::is_same_v<T, String> && !std::is_same_v<T, ByteArray> && HasToUtf8<T>::value>
{
    auto utf8 = val.toUtf8();
    dest.append(utf8.constData(), utf8.size());
}

template<typename T>
inline auto logFormat(ByteArray & dest, const T & val)
    -> std::enable_if_t<!std::is_same_v<T, ByteArray> && !std::is_same_v<T, String>
        && !HasToUtf8<T>::value && HasCharConstData<T>::value>
{
    dest.append(val.constData(), val.size());
}

// Fallback for types with toString() returning String-like (has c_str())
template<typename T>
inline auto logFormat(ByteArray & dest, const T & val)
    -> std::enable_if_t<!std::is_same_v<T, ByteArray> && !std::is_same_v<T, String>
        && !HasToUtf8<T>::value && !HasCharConstData<T>::value && HasToStringCStr<T>::value>
{
    dest.append(val.toString().c_str());
}

// Fallback for types with toString() returning QString-like (has toUtf8(), no c_str())
template<typename T>
inline auto logFormat(ByteArray & dest, const T & val)
    -> std::enable_if_t<!std::is_same_v<T, ByteArray> && !std::is_same_v<T, String>
        && !HasToUtf8<T>::value && !HasCharConstData<T>::value && !HasToStringCStr<T>::value && HasToStringToUtf8<T>::value>
{
    auto utf8 = val.toString().toUtf8();
    dest.append(utf8.constData(), utf8.size());
}

} // namespace

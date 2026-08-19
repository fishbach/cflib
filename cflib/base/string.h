/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base/bytearray.h>
#include <cflib/base/container.h>

#include <cstring>
#include <string>
#include <string_view>
#include <vector>

namespace cflib::base {

class String;
using StringList = List<String>;

// A String is UTF-8 encoded text. Since UTF-8 text is a byte sequence,
// String is a ByteArray with text-specific operations layered on top.
// Everything that operates on bytes (append, indexOf, replace, Base64, Hex,
// ...) is inherited from ByteArray; only methods with text semantics or a
// String-typed result are redeclared here.
//
// Note: size() / length() count bytes, not Unicode characters. Use
// charCount() for the number of codepoints.

class String : public ByteArray
{
public:
    String() : ByteArray() {}
    String(const char * utf8) : ByteArray(utf8) {}
    String(const char * utf8, size_t len) : ByteArray(utf8, len) {}
    String(std::string s) : ByteArray(std::move(s)) {}
    String(size_t n, char c) : ByteArray(n, c) {}
    String(std::string_view sv) : ByteArray(sv) {}
    String(const ByteArray & ba) : ByteArray(ba) {}

    // A null string stays null when resized to zero, otherwise behave like
    // ByteArray::resize (which detaches and clears the null flag).
    void resize(size_t n) { if (isNull() && n == 0) return; ByteArray::resize(n); }
    using ByteArray::resize;

    const std::string & str()      const { return toStdString(); }
    const char *        c_str()    const { return constData(); }
    size_t              byteSize() const { return size(); }

    // O(1): the UTF-8 byte sequence, shared (copy-on-write)
    ByteArray toUtf8()   const { return *this; }
    ByteArray toLatin1() const { return toUtf8(); }

    // Number of Unicode codepoints (UTF-8 scan, O(n))
    size_t charCount() const {
        size_t count = 0;
        for (size_t i = 0; i < (size_t)d->data.size(); ) {
            uint8 c = (uint8)d->data[i];
            if      (c < 0x80) i += 1;
            else if (c < 0xE0) i += 2;
            else if (c < 0xF0) i += 3;
            else               i += 4;
            ++count;
        }
        return count;
    }

    ssize_t lastIndexOf(const char * s) const {
        size_t pos = d->data.rfind(s);
        return pos == std::string::npos ? -1 : (size_t)pos;
    }

    size_t count(const String & subStr) const {
        const std::string & sub = subStr.d->data;
        size_t rv = 0;
        size_t pos = 0;
        while ((pos = d->data.find(sub, pos)) != std::string::npos) {
            ++rv;
            pos += sub.size();
        }
        return rv;
    }

    String mid(size_t bytePos, size_t len = npos) const {
        if (bytePos >= (size_t)d->data.size()) return String();
        return String(d->data.substr(bytePos, len));
    }
    String left(size_t n) const { return mid(0, n); }
    String right(size_t n) const {
        if (n >= (size_t)d->data.size()) return *this;
        return mid(d->data.size() - n);
    }

    String trimmed() const {
        size_t s = d->data.find_first_not_of(" \t\r\n");
        if (s == std::string::npos) return String();
        size_t e = d->data.find_last_not_of(" \t\r\n");
        return String(d->data.substr(s, e - s + 1));
    }

    String toLower() const {
        std::string r = d->data;
        for (char & c : r) if (c >= 'A' && c <= 'Z') c += 32;
        return String(std::move(r));
    }
    String toUpper() const {
        std::string r = d->data;
        for (char & c : r) if (c >= 'a' && c <= 'z') c -= 32;
        return String(std::move(r));
    }

    StringList split(char delim) const {
        StringList result;
        size_t start = 0, pos;
        while ((pos = d->data.find(delim, start)) != std::string::npos) {
            result << String(d->data.substr(start, pos - start));
            start = pos + 1;
        }
        result << String(d->data.substr(start));
        return result;
    }
    StringList split(const char * delim) const {
        StringList result;
        size_t dlen = strlen(delim);
        size_t start = 0, pos;
        while ((pos = d->data.find(delim, start)) != std::string::npos) {
            result << String(d->data.substr(start, pos - start));
            start = pos + dlen;
        }
        result << String(d->data.substr(start));
        return result;
    }

    String join(const StringList & list) const {
        std::string r;
        for (size_t i = 0; i < (size_t)list.size(); ++i) {
            if (i > 0) r += d->data;
            r += list[i].d->data;
        }
        return String(std::move(r));
    }

    static String fromUtf8(const char * s, size_t len = npos) {
        if (!s) return String();
        return String(len == npos ? std::string(s) : std::string(s, len));
    }
    static String fromUtf8(const ByteArray & ba) { return String(ba); }
    static String fromLatin1(const char * s) { return s ? String(s) : String(); }

    bool operator<= (const String & o) const { return d->data <= o.d->data; }
    bool operator>  (const String & o) const { return d->data >  o.d->data; }
    bool operator>= (const String & o) const { return d->data >= o.d->data; }

    String   operator+ (const String & o) const { return String(d->data + o.d->data); }
    String   operator+ (const char * s)   const { return String(d->data + s); }
    String   operator+ (char c)            const { String r(*this); r += c; return r; }
    String & operator+=(const String & o)  { detach(); d->data += o.d->data; d->isNull = false; return *this; }
    String & operator+=(const char * s)    { detach(); d->data += s;       d->isNull = false; return *this; }
    String & operator+=(char c)            { detach(); d->data += c;       d->isNull = false; return *this; }

    String & operator<<(const String & o)    { detach(); return *this += o; }
    String & operator<<(const ByteArray & o) { detach(); d->data += o.toStdString(); d->isNull = false; return *this; }
    String & operator<<(const char * s)      { detach(); return *this += s; }
    String & operator<<(char c)              { detach(); return *this += c; }
};

// The bodies must not call `rhs == lhs` / `rhs != lhs`: under C++20
// rewritten-candidate rules that expression would resolve back to these
// free functions themselves (infinite recursion). Comparing the underlying
// std::string directly keeps them recursion-proof and mirrors the
// null-aware member semantics of ByteArray.
inline bool operator==(const char * lhs, const String & rhs) {
    return lhs ? rhs.str() == lhs : rhs.isNull();
}
inline bool operator!=(const char * lhs, const String & rhs) {
    return lhs ? !(rhs.str() == lhs) : !rhs.isNull();
}

inline String operator+(const char * lhs, const String & rhs) {
    return String(std::string(lhs) + rhs.str());
}

} // namespace

namespace std {
template<> struct hash<cflib::base::String> {
    size_t operator()(const cflib::base::String & s) const {
        return hash<string>()(s.str());
    }
};
}

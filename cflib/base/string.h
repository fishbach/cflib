/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base/bytearray.h>
#include <cflib/base/types.h>

#include <cstring>
#include <format>
#include <string>
#include <string_view>
#include <vector>

namespace cflib::base {

class String
{
public:
    static constexpr size_t npos = std::string::npos;

    String() : d(new Shared) {}
    String(const char * utf8) : d(new Shared{utf8 == nullptr, utf8 ? utf8 : ""}) {}
    String(const char * utf8, size_t len) : d(new Shared{false, std::string(utf8, len)}) {}
    String(std::string s) : d(new Shared{false, std::move(s)}) {}
    String(size_t n, char c) : d(new Shared{false, std::string(n, c)}) {}
    String(std::string_view sv) : d(new Shared{false, std::string(sv)}) {}
    String(const ByteArray & ba) : d(new Shared{ba.isNull(), ba.constData()}) {}

    // implicit sharing
    String(const String & other) : d(other.d) { d->ref.ref(); }
    String(String && other) : d(other.d) { other.d = new Shared; }
    ~String() { if (!d->ref.deref()) delete d; }
    String & operator=(const String & other) {
        if (d == other.d) return *this;
        if (!d->ref.deref()) delete d;
        d = other.d;
        d->ref.ref();
        return *this;
    }
    String& operator=(String && other) {
        std::swap(d, other.d);
        return *this;
    }
    void detach() {
        if (d->ref.loadAcquire() == 1) return;
        Shared * newD = new Shared{d->isNull, d->data};
        if (!d->ref.deref()) delete d;
        d = newD;
    }

    const std::string & str()    const { return d->data; }
    const char *        c_str()  const { return d->data.c_str(); }
    size_t            byteSize() const { return (size_t)d->data.size(); }
    size_t            size()   const { return (size_t)d->data.size(); }
    size_t            length() const { return (size_t)d->data.size(); }
    bool                isEmpty() const { return d->data.empty(); }
    bool                isNull()  const { return d->isNull; }

    ByteArray toUtf8()  const { return ByteArray(d->data.data(), (size_t)d->data.size()); }
    ByteArray toLatin1() const { return toUtf8(); }

    // Codepoint count
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

    ssize_t indexOf(const char * s, size_t from = 0) const {
        size_t pos = d->data.find(s, from);
        return pos == std::string::npos ? -1 : (size_t)pos;
    }
    ssize_t indexOf(const String & s, size_t from = 0) const {
        size_t pos = d->data.find(s.d->data, from);
        return pos == std::string::npos ? -1 : (size_t)pos;
    }
    ssize_t indexOf(char c, size_t from = 0) const {
        size_t pos = d->data.find(c, from);
        return pos == std::string::npos ? -1 : (size_t)pos;
    }
    ssize_t lastIndexOf(const char * s) const {
        size_t pos = d->data.rfind(s);
        return pos == std::string::npos ? -1 : (size_t)pos;
    }

    bool contains(const char * s) const { return d->data.find(s) != std::string::npos; }
    bool contains(char c) const { return d->data.find(c) != std::string::npos; }
    bool startsWith(const char * s) const { return d->data.rfind(s, 0) == 0; }
    bool startsWith(const String & s) const { return d->data.rfind(s.d->data, 0) == 0; }
    bool endsWith(const char * s) const {
        size_t slen = strlen(s);
        if (slen > (size_t)d->data.size()) return false;
        return d->data.compare(d->data.size() - slen, slen, s) == 0;
    }
    bool endsWith(const String & s) const {
        if (s.d->data.size() > d->data.size()) return false;
        return d->data.compare(d->data.size() - s.d->data.size(), s.d->data.size(), s.d->data) == 0;
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

    std::vector<String> split(char delim) const {
        std::vector<String> result;
        size_t start = 0, pos;
        while ((pos = d->data.find(delim, start)) != std::string::npos) {
            result.push_back(String(d->data.substr(start, pos - start)));
            start = pos + 1;
        }
        result.push_back(String(d->data.substr(start)));
        return result;
    }
    std::vector<String> split(const char * delim) const {
        std::vector<String> result;
        size_t dlen = strlen(delim);
        size_t start = 0, pos;
        while ((pos = d->data.find(delim, start)) != std::string::npos) {
            result.push_back(String(d->data.substr(start, pos - start)));
            start = pos + dlen;
        }
        result.push_back(String(d->data.substr(start)));
        return result;
    }

    String & replace(const char * before, const char * after) {
        detach();
        const size_t blen = strlen(before);
        const size_t alen = strlen(after);
        size_t pos = 0;
        while ((pos = d->data.find(before, pos)) != std::string::npos) {
            d->data.replace(pos, blen, after, alen);
            pos += alen;
        }
        return *this;
    }
    String & replace(size_t pos, size_t len, const char * s) {
        detach();
        d->data.replace(pos, len, s);
        return *this;
    }

    String join(const std::vector<String> & list) const {
        std::string r;
        for (size_t i = 0; i < (size_t)list.size(); ++i) {
            if (i > 0) r += d->data;
            r += list[i].d->data;
        }
        return String(std::move(r));
    }

    // Number formatting
    static String number(std::integral auto v) {
        return String(std::format("{}", v).c_str());
    }
    static String number(std::floating_point auto v) {
        return String(std::format("{:g}", v).c_str());
    }

    int64  toLong(bool * ok = nullptr) const {
        if (d->data.empty()) { if (ok) *ok = false; return 0; }
        char * end = nullptr;
        int64 v = strtoll(d->data.c_str(), &end, 10);
        if (ok) *ok = (end != d->data.c_str() && *end == '\0');
        return v;
    }
    uint64 toULong(bool * ok = nullptr) const {
        if (d->data.empty()) { if (ok) *ok = false; return 0; }
        char * end = nullptr;
        uint64 v = strtoull(d->data.c_str(), &end, 10);
        if (ok) *ok = (end != d->data.c_str() && *end == '\0');
        return v;
    }

    static String fromUtf8(const char * s, size_t len = npos) {
        if (!s) return String();
        return String(len == npos ? std::string(s) : std::string(s, len));
    }
    static String fromUtf8(const ByteArray & ba) {
        return String(std::string(ba.constData(), ba.size()));
    }
    static String fromLatin1(const char * s) { return s ? String(s) : String(); }

    bool operator==(const String & o) const { return d->data == o.d->data; }
    bool operator==(const char * o)     const { return o ? d->data == o : d->isNull; }
    bool operator!=(const String & o) const { return d->data != o.d->data; }
    bool operator!=(const char * o)     const { return !((*this) == o); }
    bool operator< (const String & o) const { return d->data <  o.d->data; }
    bool operator<=(const String & o) const { return d->data <= o.d->data; }
    bool operator> (const String & o) const { return d->data >  o.d->data; }
    bool operator>=(const String & o) const { return d->data >= o.d->data; }

    String   operator+ (const String & o) const { return String(d->data + o.d->data); }
    String   operator+ (const char * s)   const { return String(d->data + s); }
    String & operator+=(const String & o)       { detach(); d->data += o.d->data; d->isNull = false; return *this; }
    String & operator+=(const char * s)         { detach(); d->data += s;       d->isNull = false; return *this; }
    String & operator+=(char c)                 { detach(); d->data += c;       d->isNull = false; return *this; }

    String & operator<<(const String & o) { detach(); return *this += o; }
    String & operator<<(const char * s)   { detach(); return *this += s; }
    String & operator<<(char c)           { detach(); return *this += c; }

    void clear() { detach(); d->data.clear(); d->isNull = true; }
    void remove(size_t pos, size_t len) { detach(); d->data.erase(pos, len); }

    char operator[](size_t i) const { return d->data[i]; }
    char & operator[](size_t i) { detach(); return d->data[i]; }

    uint32 toUInt(bool * ok = nullptr) const {
        if (d->data.empty()) { if (ok) *ok = false; return 0; }
        char * end = nullptr;
        unsigned long v = strtoul(d->data.c_str(), &end, 10);
        if (ok) *ok = (end != d->data.c_str() && *end == '\0');
        return (uint32)v;
    }

    int32 toInt(bool * ok = nullptr) const {
        if (d->data.empty()) { if (ok) *ok = false; return 0; }
        char * end = nullptr;
        long v = strtol(d->data.c_str(), &end, 10);
        if (ok) *ok = (end != d->data.c_str() && *end == '\0');
        return (int32)v;
    }

    String simplified() const;

private:
    struct Shared
    {
        bool isNull = true;
        std::string data;
        AtomicInt ref{1};
    };
    Shared * d;
};

inline bool operator==(const char * lhs, const String & rhs) { return rhs == lhs; }
inline bool operator!=(const char * lhs, const String & rhs) { return rhs != lhs; }

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

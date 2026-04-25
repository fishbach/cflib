/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base/concurrent.h>
#include <cflib/base/types.h>

#include <concepts>
#include <cstring>
#include <format>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace cflib::base {

class ByteArray
{
public:
    static constexpr size_t npos = std::string::npos;

    ByteArray() : d(new Shared) {}
    ByteArray(const char * data) : d(new Shared{data == nullptr, data ? data : ""}) {}
    ByteArray(const char * data, size_t len) : d(new Shared{false, std::string(data, len)}) {}
    ByteArray(const uint8 * data, size_t len) : d(new Shared{false, std::string((const char *)data, len)}) {}
    ByteArray(size_t n, char c) : d(new Shared{false, std::string(n, c)}) {}
    ByteArray(std::string_view sv) : d(new Shared{false, std::string(sv)}) {}
    explicit ByteArray(std::string s) : d(new Shared{false, std::move(s)}) {}

    // implicit sharing
    ByteArray(const ByteArray & other) : d(other.d) { d->ref.ref(); }
    ByteArray(ByteArray && other) : d(other.d) { other.d = new Shared; }
    ~ByteArray() { if (!d->ref.deref()) delete d; }
    ByteArray & operator=(const ByteArray & other) {
        if (d == other.d) return *this;
        if (!d->ref.deref()) delete d;
        d = other.d;
        d->ref.ref();
        return *this;
    }
    ByteArray& operator=(ByteArray && other) {
        std::swap(d, other.d);
        return *this;
    }
    void detach() {
        if (d->ref.loadAcquire() == 1) return;
        Shared * newD = new Shared{d->isNull, d->data};
        if (!d->ref.deref()) delete d;
        d = newD;
    }

    static ByteArray fromRawData(const char * data, size_t len) { return ByteArray(data, len); }

    char *       data()       { detach(); return d->data.data(); }
    const char * data() const { return d->data.data(); }
    const char * constData() const { return d->data.data(); }

    size_t size()   const { return (size_t)d->data.size(); }
    size_t length() const { return (size_t)d->data.size(); }
    bool     isEmpty() const { return d->data.empty(); }
    bool     isNull() const { return d->isNull; }

    void resize(size_t n)         { detach(); d->data.resize(n); d->isNull = false; }
    void resize(size_t n, char c) { detach(); d->data.resize(n, c); d->isNull = false; }
    void reserve(size_t n)        { detach(); d->data.reserve(n); }
    void clear()                  { detach(); d->data.clear(); d->isNull = true; }
    size_t capacity() const       { return (size_t)d->data.capacity(); }

    char   operator[](size_t i) const { return d->data[i]; }
    char & operator[](size_t i)       { detach(); return d->data[i]; }
    char   at(size_t i) const         { return d->data[i]; }

    ByteArray & append(char c)                      { detach(); d->data += c; d->isNull = false; return *this; }
    ByteArray & append(const char * s)              { detach(); d->data += s; d->isNull = false; return *this; }
    ByteArray & append(const char * s, size_t len)  { detach(); d->data.append(s, len); d->isNull = false; return *this; }
    ByteArray & append(const ByteArray & other)     { detach(); d->data += other.d->data; d->isNull = false; return *this; }

    ByteArray & prepend(const char * s, size_t len) { detach(); d->data.insert(0, s, len); d->isNull = false; return *this; }
    ByteArray & prepend(const char * s)             { detach(); d->data.insert(0, s); d->isNull = false; return *this; }

    ByteArray & insert(size_t pos, const ByteArray & ba) {
        detach();
        d->data.insert(pos, ba.d->data);
        d->isNull = false;
        return *this;
    }
    ByteArray & insert(size_t pos, const char * s, size_t len) {
        detach();
        d->data.insert(pos, s, len);
        d->isNull = false;
        return *this;
    }

    ByteArray mid(size_t pos, size_t len = npos) const {
        if (pos >= (size_t)d->data.size()) return ByteArray();
        return ByteArray(d->data.substr(pos, len));
    }
    ByteArray left(size_t n) const  { return mid(0, n); }
    ByteArray right(size_t n) const {
        if (n >= (size_t)d->data.size()) return *this;
        return mid(d->data.size() - n);
    }

    bool startsWith(const char * s) const { return d->data.rfind(s, 0) == 0; }
    bool startsWith(const ByteArray & other) const { return d->data.rfind(other.d->data, 0) == 0; }
    bool endsWith(const char * s) const {
        size_t slen = strlen(s);
        if (slen > (size_t)d->data.size()) return false;
        return d->data.compare(d->data.size() - slen, slen, s) == 0;
    }
    bool endsWith(const ByteArray & other) const {
        if (other.d->data.size() > d->data.size()) return false;
        return d->data.compare(d->data.size() - other.d->data.size(), other.d->data.size(), other.d->data) == 0;
    }
    bool contains(const char * s) const { return d->data.find(s) != std::string::npos; }
    bool contains(char c) const { return d->data.find(c) != std::string::npos; }

    ssize_t indexOf(char c, size_t from = 0) const {
        size_t pos = d->data.find(c, from);
        return pos == std::string::npos ? -1 : (size_t)pos;
    }
    ssize_t indexOf(const char * s, size_t from = 0) const {
        size_t pos = d->data.find(s, from);
        return pos == std::string::npos ? -1 : (size_t)pos;
    }
    ssize_t indexOf(const ByteArray & other, size_t from = 0) const {
        size_t pos = d->data.find(other.d->data, from);
        return pos == std::string::npos ? -1 : (size_t)pos;
    }

    // replace(pos, len, newData, newLen) -- in-place substitution
    ByteArray & replace(size_t pos, size_t len, const char * newData, size_t newLen) {
        detach();
        d->data.replace(pos, len, newData, newLen);
        return *this;
    }
    ByteArray & replace(size_t pos, size_t len, const char * newData) {
        detach();
        d->data.replace(pos, len, newData);
        return *this;
    }

    ByteArray & replace(const char * before, const char * after) {
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
    ByteArray & replace(char before, const char * after) {
        detach();
        const size_t alen = strlen(after);
        size_t pos = 0;
        while ((pos = d->data.find(before, pos)) != std::string::npos) {
            d->data.replace(pos, 1, after, alen);
            pos += alen;
        }
        return *this;
    }

    // Numeric conversions
    uint32 toUInt(bool * ok = nullptr) const;
    int32 toInt(bool * ok = nullptr) const;

    ByteArray trimmed() const {
        size_t s = d->data.find_first_not_of(" \t\r\n");
        if (s == std::string::npos) return ByteArray();
        size_t e = d->data.find_last_not_of(" \t\r\n");
        return ByteArray(d->data.substr(s, e - s + 1));
    }

    uint64 toULongLong(bool * ok = nullptr) const {
        if (d->data.empty()) { if (ok) *ok = false; return 0; }
        char * end = nullptr;
        unsigned long long v = strtoull(d->data.c_str(), &end, 10);
        if (ok) *ok = (end != d->data.c_str() && *end == '\0');
        return (uint64)v;
    }

    ByteArray toLower() const {
        std::string r = d->data;
        for (char & c : r) if (c >= 'A' && c <= 'Z') c += 32;
        return ByteArray(std::move(r));
    }

    // split by character
    std::vector<ByteArray> split(char delim) const {
        std::vector<ByteArray> result;
        size_t start = 0, pos;
        while ((pos = d->data.find(delim, start)) != std::string::npos) {
            result.push_back(ByteArray(d->data.substr(start, pos - start)));
            start = pos + 1;
        }
        result.push_back(ByteArray(d->data.substr(start)));
        return result;
    }

    // remove bytes at position
    ByteArray & remove(size_t pos, size_t len) {
        detach();
        d->data.erase(pos, len);
        return *this;
    }

    // Base64 encoding/decoding
    ByteArray toBase64() const {
        static const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string out;
        out.reserve(((d->data.size() + 2) / 3) * 4);
        size_t i = 0;
        while (i + 2 < d->data.size()) {
            uint32 n = ((uint8)d->data[i] << 16) | ((uint8)d->data[i+1] << 8) | (uint8)d->data[i+2];
            out += table[(n >> 18) & 0x3F];
            out += table[(n >> 12) & 0x3F];
            out += table[(n >>  6) & 0x3F];
            out += table[n & 0x3F];
            i += 3;
        }
        if (i + 1 == d->data.size()) {
            uint32 n = (uint8)d->data[i] << 16;
            out += table[(n >> 18) & 0x3F];
            out += table[(n >> 12) & 0x3F];
            out += '=';
            out += '=';
        } else if (i + 2 == d->data.size()) {
            uint32 n = ((uint8)d->data[i] << 16) | ((uint8)d->data[i+1] << 8);
            out += table[(n >> 18) & 0x3F];
            out += table[(n >> 12) & 0x3F];
            out += table[(n >>  6) & 0x3F];
            out += '=';
        }
        return ByteArray(std::move(out));
    }

    static ByteArray fromBase64(const ByteArray & base64) {
        auto decode = [](char c) -> int {
            if (c >= 'A' && c <= 'Z') return c - 'A';
            if (c >= 'a' && c <= 'z') return c - 'a' + 26;
            if (c >= '0' && c <= '9') return c - '0' + 52;
            if (c == '+') return 62;
            if (c == '/') return 63;
            return -1;
        };
        std::string out;
        out.reserve(base64.size() * 3 / 4);
        int buf = 0, bits = 0;
        for (size_t i = 0; i < base64.size(); ++i) {
            int v = decode(base64[i]);
            if (v < 0) continue;
            buf = (buf << 6) | v;
            bits += 6;
            if (bits >= 8) {
                bits -= 8;
                out += (char)((buf >> bits) & 0xFF);
            }
        }
        return ByteArray(std::move(out));
    }

    const std::string & toStdString() const { return d->data; }
    std::string       & toStdString()       { detach(); return d->data; }

    // Hex conversion
    static ByteArray fromHex(const char * hex) {
        ByteArray out;
        size_t len = strlen(hex);
        out.reserve(len / 2);
        auto hexVal = [](char c) -> int {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            return -1;
        };
        size_t i = 0;
        while (i < len) {
            while (i < len && (hex[i] == ' ' || hex[i] == '\t' || hex[i] == '\n' || hex[i] == '\r')) ++i;
            if (i + 1 >= len) break;
            int hi = hexVal(hex[i]), lo = hexVal(hex[i+1]);
            if (hi < 0 || lo < 0) break;
            out += (char)((hi << 4) | lo);
            i += 2;
        }
        return out;
    }
    static ByteArray fromHex(const ByteArray & hex) { return fromHex(hex.constData()); }

    ByteArray toHex() const {
        static const char * digits = "0123456789abcdef";
        ByteArray out((size_t)(d->data.size() * 2), '\0');
        for (size_t i = 0; i < (size_t)d->data.size(); ++i) {
            uint8 c = (uint8)d->data[i];
            out.d->data[i*2]   = digits[c >> 4];
            out.d->data[i*2+1] = digits[c & 0xF];
        }
        return out;
    }

    // Number formatting
    static ByteArray number(std::integral auto v) {
        return ByteArray(std::format("{}", v).c_str());
    }
    static ByteArray number(std::floating_point auto v) {
        return ByteArray(std::format("{:g}", v).c_str());
    }

    ByteArray & operator+=(char c)              { detach(); d->data += c; d->isNull = false; return *this; }
    ByteArray & operator+=(const char * s)      { detach(); d->data += s; d->isNull = false; return *this; }
    ByteArray & operator+=(const ByteArray & o) { detach(); d->data += o.d->data; d->isNull = false; return *this; }

    ByteArray operator+(char c)              const { ByteArray r(*this); r += c; return r; }
    ByteArray operator+(const char * s)      const { ByteArray r(*this); r += s; return r; }
    ByteArray operator+(const ByteArray & o) const { ByteArray r(*this); r += o; return r; }

    bool operator==(const ByteArray & o) const { return d->data == o.d->data; }
    bool operator!=(const ByteArray & o) const { return d->data != o.d->data; }
    bool operator< (const ByteArray & o) const { return d->data <  o.d->data; }
    bool operator==(const char * o) const { return d->data == o; }
    bool operator!=(const char * o) const { return d->data != o; }

private:
    struct Shared
    {
        bool isNull = true;
        std::string data;
        AtomicInt ref{1};
    };
    Shared * d;
};

inline ByteArray operator+(const char * lhs, const ByteArray & rhs) {
    ByteArray r(lhs);
    r += rhs;
    return r;
}

inline ByteArray & operator<<(ByteArray & lhs, const ByteArray & rhs) { return lhs += rhs; }
inline ByteArray & operator<<(ByteArray & lhs, const char * rhs)      { return lhs += rhs; }
inline ByteArray & operator<<(ByteArray & lhs, char rhs)              { return lhs += rhs; }

} // namespace

namespace std {
template<> struct hash<cflib::base::ByteArray> {
    size_t operator()(const cflib::base::ByteArray & ba) const {
        return hash<string>()(ba.toStdString());
    }
};
}

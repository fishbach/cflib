/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base/types.h>

#include <cstdio>
#include <cstring>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

class CFByteArray
{
public:
    static constexpr cfsize_t npos = std::string::npos;

    CFByteArray() = default;
    CFByteArray(const char * data) : data_(data ? data : ""), isNull_(!data) {}
    CFByteArray(const char * data, cfsize_t len) : data_(data, len), isNull_(false) {}
    CFByteArray(cfsize_t n, char c) : data_(n, c), isNull_(false) {}
    CFByteArray(std::string_view sv) : data_(sv), isNull_(false) {}
    explicit CFByteArray(std::string s) : data_(std::move(s)), isNull_(false) {}

    // Qt compatibility: accept any type with constData() returning const char* (e.g. QByteArray)
    template<typename T, std::enable_if_t<
        !std::is_same_v<std::decay_t<T>, CFByteArray> &&
        !std::is_pointer_v<std::decay_t<T>> &&
        !std::is_same_v<std::decay_t<T>, std::string> &&
        !std::is_same_v<std::decay_t<T>, std::string_view> &&
        std::is_same_v<decltype(std::declval<const T&>().constData()), const char*>,
        int> = 0>
    CFByteArray(const T & qba) : data_(qba.constData(), qba.size()), isNull_(false) {}

    static CFByteArray fromRawData(const char * data, cfsize_t len) { return CFByteArray(data, len); }

    char *       data()       { return data_.data(); }
    const char * data() const { return data_.data(); }
    const char * constData() const { return data_.data(); }

    cfsize_t size()   const { return (cfsize_t)data_.size(); }
    cfsize_t length() const { return (cfsize_t)data_.size(); }
    bool     isEmpty() const { return data_.empty(); }
    bool     isNull() const { return isNull_; }

    void resize(cfsize_t n)         { data_.resize(n); isNull_ = false; }
    void resize(cfsize_t n, char c) { data_.resize(n, c); isNull_ = false; }
    void reserve(cfsize_t n)        { data_.reserve(n); }
    void clear()                    { data_.clear(); isNull_ = true; }
    cfsize_t capacity() const       { return (cfsize_t)data_.capacity(); }

    char   operator[](cfsize_t i) const { return data_[i]; }
    char & operator[](cfsize_t i)       { return data_[i]; }
    char   at(cfsize_t i) const         { return data_[i]; }

    CFByteArray & append(char c)                        { data_ += c; isNull_ = false; return *this; }
    CFByteArray & append(const char * s)                { data_ += s; isNull_ = false; return *this; }
    CFByteArray & append(const char * s, cfsize_t len)  { data_.append(s, len); isNull_ = false; return *this; }
    CFByteArray & append(const CFByteArray & other)     { data_ += other.data_; isNull_ = false; return *this; }

    CFByteArray & prepend(const char * s, cfsize_t len) { data_.insert(0, s, len); isNull_ = false; return *this; }
    CFByteArray & prepend(const char * s)               { data_.insert(0, s); isNull_ = false; return *this; }

    CFByteArray & insert(cfsize_t pos, const CFByteArray & ba) {
        data_.insert(pos, ba.data_);
        isNull_ = false;
        return *this;
    }
    CFByteArray & insert(cfsize_t pos, const char * s, cfsize_t len) {
        data_.insert(pos, s, len);
        isNull_ = false;
        return *this;
    }

    CFByteArray mid(cfsize_t pos, cfsize_t len = npos) const {
        if (pos >= (cfsize_t)data_.size()) return CFByteArray();
        return CFByteArray(data_.substr(pos, len));
    }
    CFByteArray left(cfsize_t n) const  { return mid(0, n); }
    CFByteArray right(cfsize_t n) const {
        if (n >= (cfsize_t)data_.size()) return *this;
        return mid(data_.size() - n);
    }

    bool startsWith(const char * s) const { return data_.rfind(s, 0) == 0; }
    bool startsWith(const CFByteArray & other) const { return data_.rfind(other.data_, 0) == 0; }
    bool endsWith(const char * s) const {
        cfsize_t slen = strlen(s);
        if (slen > (cfsize_t)data_.size()) return false;
        return data_.compare(data_.size() - slen, slen, s) == 0;
    }
    bool endsWith(const CFByteArray & other) const {
        if (other.data_.size() > data_.size()) return false;
        return data_.compare(data_.size() - other.data_.size(), other.data_.size(), other.data_) == 0;
    }
    bool contains(const char * s) const { return data_.find(s) != std::string::npos; }
    bool contains(char c) const { return data_.find(c) != std::string::npos; }

    cfsize_t indexOf(char c, cfsize_t from = 0) const {
        cfsize_t pos = data_.find(c, from);
        return pos == std::string::npos ? -1 : (cfsize_t)pos;
    }
    cfsize_t indexOf(const char * s, cfsize_t from = 0) const {
        cfsize_t pos = data_.find(s, from);
        return pos == std::string::npos ? -1 : (cfsize_t)pos;
    }
    cfsize_t indexOf(const CFByteArray & other, cfsize_t from = 0) const {
        cfsize_t pos = data_.find(other.data_, from);
        return pos == std::string::npos ? -1 : (cfsize_t)pos;
    }

    // replace(pos, len, newData, newLen) -- in-place substitution
    CFByteArray & replace(cfsize_t pos, cfsize_t len, const char * newData, cfsize_t newLen) {
        data_.replace(pos, len, newData, newLen);
        return *this;
    }
    CFByteArray & replace(cfsize_t pos, cfsize_t len, const char * newData) {
        data_.replace(pos, len, newData);
        return *this;
    }

    // replace all occurrences of before with after
    CFByteArray & replace(const char * before, const char * after) {
        const cfsize_t blen = strlen(before);
        const cfsize_t alen = strlen(after);
        cfsize_t pos = 0;
        while ((pos = data_.find(before, pos)) != std::string::npos) {
            data_.replace(pos, blen, after, alen);
            pos += alen;
        }
        return *this;
    }
    CFByteArray & replace(char before, const char * after) {
        const cfsize_t alen = strlen(after);
        cfsize_t pos = 0;
        while ((pos = data_.find(before, pos)) != std::string::npos) {
            data_.replace(pos, 1, after, alen);
            pos += alen;
        }
        return *this;
    }

    void detach() {} // no-op for std::string (always detached)

    // Numeric conversions
    cfuint32 toUInt(bool * ok = nullptr) const {
        if (data_.empty()) { if (ok) *ok = false; return 0; }
        char * end = nullptr;
        unsigned long v = strtoul(data_.c_str(), &end, 10);
        if (ok) *ok = (end != data_.c_str() && *end == '\0');
        return (cfuint32)v;
    }
    cfint32 toInt(bool * ok = nullptr) const {
        if (data_.empty()) { if (ok) *ok = false; return 0; }
        char * end = nullptr;
        long v = strtol(data_.c_str(), &end, 10);
        if (ok) *ok = (end != data_.c_str() && *end == '\0');
        return (cfint32)v;
    }

    CFByteArray trimmed() const {
        cfsize_t s = data_.find_first_not_of(" \t\r\n");
        if (s == std::string::npos) return CFByteArray();
        cfsize_t e = data_.find_last_not_of(" \t\r\n");
        return CFByteArray(data_.substr(s, e - s + 1));
    }

    cfuint64 toULongLong(bool * ok = nullptr) const {
        if (data_.empty()) { if (ok) *ok = false; return 0; }
        char * end = nullptr;
        unsigned long long v = strtoull(data_.c_str(), &end, 10);
        if (ok) *ok = (end != data_.c_str() && *end == '\0');
        return (cfuint64)v;
    }

    CFByteArray toLower() const {
        std::string r = data_;
        for (char & c : r) if (c >= 'A' && c <= 'Z') c += 32;
        return CFByteArray(std::move(r));
    }

    // split by character
    std::vector<CFByteArray> split(char delim) const {
        std::vector<CFByteArray> result;
        cfsize_t start = 0, pos;
        while ((pos = data_.find(delim, start)) != std::string::npos) {
            result.push_back(CFByteArray(data_.substr(start, pos - start)));
            start = pos + 1;
        }
        result.push_back(CFByteArray(data_.substr(start)));
        return result;
    }

    // remove bytes at position
    CFByteArray & remove(cfsize_t pos, cfsize_t len) {
        data_.erase(pos, len);
        return *this;
    }

    // Base64 encoding/decoding
    CFByteArray toBase64() const {
        static const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string out;
        out.reserve(((data_.size() + 2) / 3) * 4);
        cfsize_t i = 0;
        while (i + 2 < data_.size()) {
            cfuint32 n = ((cfuint8)data_[i] << 16) | ((cfuint8)data_[i+1] << 8) | (cfuint8)data_[i+2];
            out += table[(n >> 18) & 0x3F];
            out += table[(n >> 12) & 0x3F];
            out += table[(n >>  6) & 0x3F];
            out += table[n & 0x3F];
            i += 3;
        }
        if (i + 1 == data_.size()) {
            cfuint32 n = (cfuint8)data_[i] << 16;
            out += table[(n >> 18) & 0x3F];
            out += table[(n >> 12) & 0x3F];
            out += '=';
            out += '=';
        } else if (i + 2 == data_.size()) {
            cfuint32 n = ((cfuint8)data_[i] << 16) | ((cfuint8)data_[i+1] << 8);
            out += table[(n >> 18) & 0x3F];
            out += table[(n >> 12) & 0x3F];
            out += table[(n >>  6) & 0x3F];
            out += '=';
        }
        return CFByteArray(std::move(out));
    }

    static CFByteArray fromBase64(const CFByteArray & base64) {
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
        for (cfsize_t i = 0; i < base64.size(); ++i) {
            int v = decode(base64[i]);
            if (v < 0) continue;
            buf = (buf << 6) | v;
            bits += 6;
            if (bits >= 8) {
                bits -= 8;
                out += (char)((buf >> bits) & 0xFF);
            }
        }
        return CFByteArray(std::move(out));
    }

    const std::string & toStdString() const { return data_; }
    std::string       & toStdString()       { return data_; }

    // Hex conversion
    static CFByteArray fromHex(const char * hex) {
        CFByteArray out;
        cfsize_t len = strlen(hex);
        out.reserve(len / 2);
        auto hexVal = [](char c) -> int {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            return -1;
        };
        cfsize_t i = 0;
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
    static CFByteArray fromHex(const CFByteArray & hex) { return fromHex(hex.constData()); }

    CFByteArray toHex() const {
        static const char * digits = "0123456789abcdef";
        CFByteArray out((cfsize_t)(data_.size() * 2), '\0');
        for (cfsize_t i = 0; i < (cfsize_t)data_.size(); ++i) {
            cfuint8 c = (cfuint8)data_[i];
            out.data_[i*2]   = digits[c >> 4];
            out.data_[i*2+1] = digits[c & 0xF];
        }
        return out;
    }

    // Number formatting
    static CFByteArray number(cfint64 v) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%lld", (long long)v);
        return CFByteArray(buf);
    }
    static CFByteArray number(cfuint64 v) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%llu", (unsigned long long)v);
        return CFByteArray(buf);
    }
    static CFByteArray number(double v) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%g", v);
        return CFByteArray(buf);
    }
    static CFByteArray number(float  v) { return number((double)v); }
    static CFByteArray number(cfint32  v) { return number((cfint64)v); }
    static CFByteArray number(cfuint32 v) { return number((cfuint64)v); }

    CFByteArray & operator+=(char c)               { data_ += c; isNull_ = false; return *this; }
    CFByteArray & operator+=(const char * s)        { data_ += s; isNull_ = false; return *this; }
    CFByteArray & operator+=(const CFByteArray & o) { data_ += o.data_; isNull_ = false; return *this; }

    CFByteArray operator+(char c)               const { CFByteArray r(*this); r += c; return r; }
    CFByteArray operator+(const char * s)       const { CFByteArray r(*this); r += s; return r; }
    CFByteArray operator+(const CFByteArray & o)const { CFByteArray r(*this); r += o; return r; }

    bool operator==(const CFByteArray & o) const { return data_ == o.data_; }
    bool operator!=(const CFByteArray & o) const { return data_ != o.data_; }
    bool operator< (const CFByteArray & o) const { return data_ <  o.data_; }
    bool operator==(const char * o) const { return data_ == o; }
    bool operator!=(const char * o) const { return data_ != o; }

private:
    std::string data_;
    bool isNull_ = true;

    // grant access for CFByteArray::toHex
    friend class CFByteArray;
};

inline CFByteArray operator+(const char * lhs, const CFByteArray & rhs) {
    CFByteArray r(lhs);
    r += rhs;
    return r;
}

inline CFByteArray & operator<<(CFByteArray & lhs, const CFByteArray & rhs) { return lhs += rhs; }
inline CFByteArray & operator<<(CFByteArray & lhs, const char * rhs)        { return lhs += rhs; }
inline CFByteArray & operator<<(CFByteArray & lhs, char rhs)                { return lhs += rhs; }

namespace std {
template<> struct hash<CFByteArray> {
    size_t operator()(const CFByteArray & ba) const noexcept {
        return hash<string>()(ba.toStdString());
    }
};
}

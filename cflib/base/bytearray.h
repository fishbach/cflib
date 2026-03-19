/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base/types.h>

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

    ByteArray() = default;
    ByteArray(const char * data) : data_(data ? data : ""), isNull_(!data) {}
    ByteArray(const char * data, size_t len) : data_(data, len), isNull_(false) {}
    ByteArray(size_t n, char c) : data_(n, c), isNull_(false) {}
    ByteArray(std::string_view sv) : data_(sv), isNull_(false) {}
    explicit ByteArray(std::string s) : data_(std::move(s)), isNull_(false) {}

    template<typename T, std::enable_if_t<
        !std::is_same_v<std::decay_t<T>, ByteArray> &&
        !std::is_pointer_v<std::decay_t<T>> &&
        !std::is_same_v<std::decay_t<T>, std::string> &&
        !std::is_same_v<std::decay_t<T>, std::string_view> &&
        std::is_same_v<decltype(std::declval<const T&>().constData()), const char*>,
        int> = 0>
    ByteArray(const T & ba) : data_(ba.constData(), ba.size()), isNull_(false) {}

    static ByteArray fromRawData(const char * data, size_t len) { return ByteArray(data, len); }

    char *       data()       { return data_.data(); }
    const char * data() const { return data_.data(); }
    const char * constData() const { return data_.data(); }

    size_t size()   const { return (size_t)data_.size(); }
    size_t length() const { return (size_t)data_.size(); }
    bool     isEmpty() const { return data_.empty(); }
    bool     isNull() const { return isNull_; }

    void resize(size_t n)         { data_.resize(n); isNull_ = false; }
    void resize(size_t n, char c) { data_.resize(n, c); isNull_ = false; }
    void reserve(size_t n)        { data_.reserve(n); }
    void clear()                    { data_.clear(); isNull_ = true; }
    size_t capacity() const       { return (size_t)data_.capacity(); }

    char   operator[](size_t i) const { return data_[i]; }
    char & operator[](size_t i)       { return data_[i]; }
    char   at(size_t i) const         { return data_[i]; }

    ByteArray & append(char c)                        { data_ += c; isNull_ = false; return *this; }
    ByteArray & append(const char * s)                { data_ += s; isNull_ = false; return *this; }
    ByteArray & append(const char * s, size_t len)  { data_.append(s, len); isNull_ = false; return *this; }
    ByteArray & append(const ByteArray & other)     { data_ += other.data_; isNull_ = false; return *this; }

    ByteArray & prepend(const char * s, size_t len) { data_.insert(0, s, len); isNull_ = false; return *this; }
    ByteArray & prepend(const char * s)               { data_.insert(0, s); isNull_ = false; return *this; }

    ByteArray & insert(size_t pos, const ByteArray & ba) {
        data_.insert(pos, ba.data_);
        isNull_ = false;
        return *this;
    }
    ByteArray & insert(size_t pos, const char * s, size_t len) {
        data_.insert(pos, s, len);
        isNull_ = false;
        return *this;
    }

    ByteArray mid(size_t pos, size_t len = npos) const {
        if (pos >= (size_t)data_.size()) return ByteArray();
        return ByteArray(data_.substr(pos, len));
    }
    ByteArray left(size_t n) const  { return mid(0, n); }
    ByteArray right(size_t n) const {
        if (n >= (size_t)data_.size()) return *this;
        return mid(data_.size() - n);
    }

    bool startsWith(const char * s) const { return data_.rfind(s, 0) == 0; }
    bool startsWith(const ByteArray & other) const { return data_.rfind(other.data_, 0) == 0; }
    bool endsWith(const char * s) const {
        size_t slen = strlen(s);
        if (slen > (size_t)data_.size()) return false;
        return data_.compare(data_.size() - slen, slen, s) == 0;
    }
    bool endsWith(const ByteArray & other) const {
        if (other.data_.size() > data_.size()) return false;
        return data_.compare(data_.size() - other.data_.size(), other.data_.size(), other.data_) == 0;
    }
    bool contains(const char * s) const { return data_.find(s) != std::string::npos; }
    bool contains(char c) const { return data_.find(c) != std::string::npos; }

    size_t indexOf(char c, size_t from = 0) const {
        size_t pos = data_.find(c, from);
        return pos == std::string::npos ? -1 : (size_t)pos;
    }
    size_t indexOf(const char * s, size_t from = 0) const {
        size_t pos = data_.find(s, from);
        return pos == std::string::npos ? -1 : (size_t)pos;
    }
    size_t indexOf(const ByteArray & other, size_t from = 0) const {
        size_t pos = data_.find(other.data_, from);
        return pos == std::string::npos ? -1 : (size_t)pos;
    }

    // replace(pos, len, newData, newLen) -- in-place substitution
    ByteArray & replace(size_t pos, size_t len, const char * newData, size_t newLen) {
        data_.replace(pos, len, newData, newLen);
        return *this;
    }
    ByteArray & replace(size_t pos, size_t len, const char * newData) {
        data_.replace(pos, len, newData);
        return *this;
    }

    ByteArray & replace(const char * before, const char * after) {
        const size_t blen = strlen(before);
        const size_t alen = strlen(after);
        size_t pos = 0;
        while ((pos = data_.find(before, pos)) != std::string::npos) {
            data_.replace(pos, blen, after, alen);
            pos += alen;
        }
        return *this;
    }
    ByteArray & replace(char before, const char * after) {
        const size_t alen = strlen(after);
        size_t pos = 0;
        while ((pos = data_.find(before, pos)) != std::string::npos) {
            data_.replace(pos, 1, after, alen);
            pos += alen;
        }
        return *this;
    }

    void detach() {} // no-op for std::string (always detached)

    // Numeric conversions
    uint32 toUInt(bool * ok = nullptr) const;
    int32 toInt(bool * ok = nullptr) const;

    ByteArray trimmed() const {
        size_t s = data_.find_first_not_of(" \t\r\n");
        if (s == std::string::npos) return ByteArray();
        size_t e = data_.find_last_not_of(" \t\r\n");
        return ByteArray(data_.substr(s, e - s + 1));
    }

    uint64 toULongLong(bool * ok = nullptr) const {
        if (data_.empty()) { if (ok) *ok = false; return 0; }
        char * end = nullptr;
        unsigned long long v = strtoull(data_.c_str(), &end, 10);
        if (ok) *ok = (end != data_.c_str() && *end == '\0');
        return (uint64)v;
    }

    ByteArray toLower() const {
        std::string r = data_;
        for (char & c : r) if (c >= 'A' && c <= 'Z') c += 32;
        return ByteArray(std::move(r));
    }

    // split by character
    std::vector<ByteArray> split(char delim) const {
        std::vector<ByteArray> result;
        size_t start = 0, pos;
        while ((pos = data_.find(delim, start)) != std::string::npos) {
            result.push_back(ByteArray(data_.substr(start, pos - start)));
            start = pos + 1;
        }
        result.push_back(ByteArray(data_.substr(start)));
        return result;
    }

    // remove bytes at position
    ByteArray & remove(size_t pos, size_t len) {
        data_.erase(pos, len);
        return *this;
    }

    // Base64 encoding/decoding
    ByteArray toBase64() const {
        static const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string out;
        out.reserve(((data_.size() + 2) / 3) * 4);
        size_t i = 0;
        while (i + 2 < data_.size()) {
            uint32 n = ((uint8)data_[i] << 16) | ((uint8)data_[i+1] << 8) | (uint8)data_[i+2];
            out += table[(n >> 18) & 0x3F];
            out += table[(n >> 12) & 0x3F];
            out += table[(n >>  6) & 0x3F];
            out += table[n & 0x3F];
            i += 3;
        }
        if (i + 1 == data_.size()) {
            uint32 n = (uint8)data_[i] << 16;
            out += table[(n >> 18) & 0x3F];
            out += table[(n >> 12) & 0x3F];
            out += '=';
            out += '=';
        } else if (i + 2 == data_.size()) {
            uint32 n = ((uint8)data_[i] << 16) | ((uint8)data_[i+1] << 8);
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

    const std::string & toStdString() const { return data_; }
    std::string       & toStdString()       { return data_; }

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
        ByteArray out((size_t)(data_.size() * 2), '\0');
        for (size_t i = 0; i < (size_t)data_.size(); ++i) {
            uint8 c = (uint8)data_[i];
            out.data_[i*2]   = digits[c >> 4];
            out.data_[i*2+1] = digits[c & 0xF];
        }
        return out;
    }

    // Number formatting
    static ByteArray number(int64 v) {
        return ByteArray(std::format("{}", (long long)v).c_str());
    }
    static ByteArray number(uint64 v) {
        return ByteArray(std::format("{}", (unsigned long long)v).c_str());
    }
    static ByteArray number(double v) {
        return ByteArray(std::format("{:g}", v).c_str());
    }
    static ByteArray number(float  v) { return number((double)v); }
    static ByteArray number(int32  v) { return number((int64)v); }
    static ByteArray number(uint32 v) { return number((uint64)v); }

    ByteArray & operator+=(char c)               { data_ += c; isNull_ = false; return *this; }
    ByteArray & operator+=(const char * s)        { data_ += s; isNull_ = false; return *this; }
    ByteArray & operator+=(const ByteArray & o) { data_ += o.data_; isNull_ = false; return *this; }

    ByteArray operator+(char c)               const { ByteArray r(*this); r += c; return r; }
    ByteArray operator+(const char * s)       const { ByteArray r(*this); r += s; return r; }
    ByteArray operator+(const ByteArray & o)const { ByteArray r(*this); r += o; return r; }

    bool operator==(const ByteArray & o) const { return data_ == o.data_; }
    bool operator!=(const ByteArray & o) const { return data_ != o.data_; }
    bool operator< (const ByteArray & o) const { return data_ <  o.data_; }
    bool operator==(const char * o) const { return data_ == o; }
    bool operator!=(const char * o) const { return data_ != o; }

private:
    std::string data_;
    bool isNull_ = true;

    // grant access for ByteArray::toHex
    friend class ByteArray;
};

inline ByteArray operator+(const char * lhs, const ByteArray & rhs) {
    ByteArray r(lhs);
    r += rhs;
    return r;
}

inline ByteArray & operator<<(ByteArray & lhs, const ByteArray & rhs) { return lhs += rhs; }
inline ByteArray & operator<<(ByteArray & lhs, const char * rhs)        { return lhs += rhs; }
inline ByteArray & operator<<(ByteArray & lhs, char rhs)                { return lhs += rhs; }

} // namespace

namespace std {
template<> struct hash<cflib::base::ByteArray> {
    size_t operator()(const cflib::base::ByteArray & ba) const noexcept {
        return hash<string>()(ba.toStdString());
    }
};
}

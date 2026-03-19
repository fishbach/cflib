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
    static constexpr cfsize_t npos = std::string::npos;

    String() noexcept = default;
    String(const char * utf8) : data_(utf8 ? utf8 : ""), isNull_(!utf8) {}
    String(std::string s) : data_(std::move(s)), isNull_(false) {}
    String(std::string_view sv) : data_(sv), isNull_(false) {}
    String(const ByteArray & ba) : data_(ba.constData(), ba.size()), isNull_(false) {}

    template<typename T, std::enable_if_t<
        !std::is_same_v<std::decay_t<T>, String> &&
        !std::is_same_v<std::decay_t<T>, ByteArray> &&
        !std::is_pointer_v<std::decay_t<T>> &&
        !std::is_same_v<std::decay_t<T>, std::string> &&
        !std::is_same_v<std::decay_t<T>, std::string_view>,
        int> = 0,
        typename = decltype(std::declval<const T&>().toUtf8().constData())>
    String(const T & str) {
        auto utf8 = str.toUtf8();
        data_ = std::string(utf8.constData(), utf8.size());
        isNull_ = false;
    }

    const std::string & str()    const noexcept { return data_; }
    const char *        c_str()  const noexcept { return data_.c_str(); }
    cfsize_t            byteSize() const noexcept { return (cfsize_t)data_.size(); }
    cfsize_t            size()   const noexcept { return (cfsize_t)data_.size(); }
    cfsize_t            length() const noexcept { return (cfsize_t)data_.size(); }
    bool                isEmpty() const noexcept { return data_.empty(); }
    bool                isNull()  const noexcept { return isNull_; }

    ByteArray toUtf8()  const { return ByteArray(data_.data(), (cfsize_t)data_.size()); }
    ByteArray toLatin1() const { return toUtf8(); }

    // Codepoint count
    cfsize_t charCount() const {
        cfsize_t count = 0;
        for (cfsize_t i = 0; i < (cfsize_t)data_.size(); ) {
            cfuint8 c = (cfuint8)data_[i];
            if      (c < 0x80) i += 1;
            else if (c < 0xE0) i += 2;
            else if (c < 0xF0) i += 3;
            else               i += 4;
            ++count;
        }
        return count;
    }

    cfsize_t indexOf(const char * s, cfsize_t from = 0) const {
        cfsize_t pos = data_.find(s, from);
        return pos == std::string::npos ? -1 : (cfsize_t)pos;
    }
    cfsize_t indexOf(const String & s, cfsize_t from = 0) const {
        cfsize_t pos = data_.find(s.data_, from);
        return pos == std::string::npos ? -1 : (cfsize_t)pos;
    }
    cfsize_t indexOf(char c, cfsize_t from = 0) const {
        cfsize_t pos = data_.find(c, from);
        return pos == std::string::npos ? -1 : (cfsize_t)pos;
    }
    cfsize_t lastIndexOf(const char * s) const {
        cfsize_t pos = data_.rfind(s);
        return pos == std::string::npos ? -1 : (cfsize_t)pos;
    }

    bool contains(const char * s) const { return data_.find(s) != std::string::npos; }
    bool contains(char c) const { return data_.find(c) != std::string::npos; }
    bool startsWith(const char * s) const { return data_.rfind(s, 0) == 0; }
    bool startsWith(const String & s) const { return data_.rfind(s.data_, 0) == 0; }
    bool endsWith(const char * s) const {
        cfsize_t slen = strlen(s);
        if (slen > (cfsize_t)data_.size()) return false;
        return data_.compare(data_.size() - slen, slen, s) == 0;
    }
    bool endsWith(const String & s) const {
        if (s.data_.size() > data_.size()) return false;
        return data_.compare(data_.size() - s.data_.size(), s.data_.size(), s.data_) == 0;
    }

    String mid(cfsize_t bytePos, cfsize_t len = npos) const {
        if (bytePos >= (cfsize_t)data_.size()) return String();
        return String(data_.substr(bytePos, len));
    }
    String left(cfsize_t n) const { return mid(0, n); }
    String right(cfsize_t n) const {
        if (n >= (cfsize_t)data_.size()) return *this;
        return mid(data_.size() - n);
    }

    String trimmed() const {
        cfsize_t s = data_.find_first_not_of(" \t\r\n");
        if (s == std::string::npos) return String();
        cfsize_t e = data_.find_last_not_of(" \t\r\n");
        return String(data_.substr(s, e - s + 1));
    }

    String toLower() const {
        std::string r = data_;
        for (char & c : r) if (c >= 'A' && c <= 'Z') c += 32;
        return String(std::move(r));
    }
    String toUpper() const {
        std::string r = data_;
        for (char & c : r) if (c >= 'a' && c <= 'z') c -= 32;
        return String(std::move(r));
    }

    std::vector<String> split(char delim) const {
        std::vector<String> result;
        cfsize_t start = 0, pos;
        while ((pos = data_.find(delim, start)) != std::string::npos) {
            result.push_back(String(data_.substr(start, pos - start)));
            start = pos + 1;
        }
        result.push_back(String(data_.substr(start)));
        return result;
    }
    std::vector<String> split(const char * delim) const {
        std::vector<String> result;
        cfsize_t dlen = strlen(delim);
        cfsize_t start = 0, pos;
        while ((pos = data_.find(delim, start)) != std::string::npos) {
            result.push_back(String(data_.substr(start, pos - start)));
            start = pos + dlen;
        }
        result.push_back(String(data_.substr(start)));
        return result;
    }

    String & replace(const char * before, const char * after) {
        const cfsize_t blen = strlen(before);
        const cfsize_t alen = strlen(after);
        cfsize_t pos = 0;
        while ((pos = data_.find(before, pos)) != std::string::npos) {
            data_.replace(pos, blen, after, alen);
            pos += alen;
        }
        return *this;
    }
    String & replace(cfsize_t pos, cfsize_t len, const char * s) {
        data_.replace(pos, len, s);
        return *this;
    }

    String join(const std::vector<String> & list) const {
        std::string r;
        for (cfsize_t i = 0; i < (cfsize_t)list.size(); ++i) {
            if (i > 0) r += data_;
            r += list[i].data_;
        }
        return String(std::move(r));
    }

    static String number(cfint64 v) {
        return String(std::format("{}", (long long)v).c_str());
    }
    static String number(cfuint64 v) {
        return String(std::format("{}", (unsigned long long)v).c_str());
    }
    static String number(double v) {
        return String(std::format("{:g}", v).c_str());
    }
    static String number(cfint32  v) { return number((cfint64)v); }
    static String number(cfuint32 v) { return number((cfuint64)v); }

    cfint64  toLong(bool * ok = nullptr) const noexcept {
        if (data_.empty()) { if (ok) *ok = false; return 0; }
        char * end = nullptr;
        cfint64 v = strtoll(data_.c_str(), &end, 10);
        if (ok) *ok = (end != data_.c_str() && *end == '\0');
        return v;
    }
    cfuint64 toULong(bool * ok = nullptr) const noexcept {
        if (data_.empty()) { if (ok) *ok = false; return 0; }
        char * end = nullptr;
        cfuint64 v = strtoull(data_.c_str(), &end, 10);
        if (ok) *ok = (end != data_.c_str() && *end == '\0');
        return v;
    }

    static String fromUtf8(const char * s, cfsize_t len = npos) {
        if (!s) return String();
        return String(len == npos ? std::string(s) : std::string(s, len));
    }
    static String fromUtf8(const ByteArray & ba) {
        return String(std::string(ba.constData(), ba.size()));
    }
    static String fromLatin1(const char * s) { return s ? String(s) : String(); }

    bool operator==(const String & o) const noexcept { return data_ == o.data_; }
    bool operator==(const char * o)     const noexcept { return o ? data_ == o : isNull_; }
    bool operator!=(const String & o) const noexcept { return data_ != o.data_; }
    bool operator!=(const char * o)     const noexcept { return !((*this) == o); }
    bool operator< (const String & o) const noexcept { return data_ <  o.data_; }
    bool operator<=(const String & o) const noexcept { return data_ <= o.data_; }
    bool operator> (const String & o) const noexcept { return data_ >  o.data_; }
    bool operator>=(const String & o) const noexcept { return data_ >= o.data_; }

    String   operator+ (const String & o) const { return String(data_ + o.data_); }
    String   operator+ (const char * s)     const { return String(data_ + s); }
    String & operator+=(const String & o) { data_ += o.data_; isNull_ = false; return *this; }
    String & operator+=(const char * s)     { data_ += s;       isNull_ = false; return *this; }
    String & operator+=(char c)             { data_ += c;       isNull_ = false; return *this; }

    String & operator<<(const String & o) { return *this += o; }
    String & operator<<(const char * s)     { return *this += s; }
    String & operator<<(char c)             { return *this += c; }

    void clear() { data_.clear(); isNull_ = true; }
    void remove(cfsize_t pos, cfsize_t len) { data_.erase(pos, len); }

    char operator[](cfsize_t i) const { return data_[i]; }
    char & operator[](cfsize_t i) { return data_[i]; }

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

    String simplified() const {
        std::string r;
        bool lastWasSpace = true;
        for (char c : data_) {
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
                if (!lastWasSpace) { r += ' '; lastWasSpace = true; }
            } else { r += c; lastWasSpace = false; }
        }
        if (!r.empty() && r.back() == ' ') r.pop_back();
        return String(std::move(r));
    }

private:
    std::string data_;
    bool isNull_ = true;
};

inline bool operator==(const char * lhs, const String & rhs) { return rhs == lhs; }
inline bool operator!=(const char * lhs, const String & rhs) { return rhs != lhs; }

inline String operator+(const char * lhs, const String & rhs) {
    return String(std::string(lhs) + rhs.str());
}

} // namespace

namespace std {
template<> struct hash<cflib::base::String> {
    size_t operator()(const cflib::base::String & s) const noexcept {
        return hash<string>()(s.str());
    }
};
}

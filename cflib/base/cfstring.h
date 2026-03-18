/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base/cfbytearray.h>
#include <cflib/base/types.h>

#include <cstring>
#include <format>
#include <string>
#include <string_view>
#include <vector>

class CFString
{
public:
    static constexpr cfsize_t npos = std::string::npos;

    CFString() noexcept = default;
    CFString(const char * utf8) : data_(utf8 ? utf8 : ""), isNull_(!utf8) {}
    CFString(std::string s) : data_(std::move(s)), isNull_(false) {}
    CFString(std::string_view sv) : data_(sv), isNull_(false) {}
    CFString(const CFByteArray & ba) : data_(ba.constData(), ba.size()), isNull_(false) {}

    template<typename T, std::enable_if_t<
        !std::is_same_v<std::decay_t<T>, CFString> &&
        !std::is_same_v<std::decay_t<T>, CFByteArray> &&
        !std::is_pointer_v<std::decay_t<T>> &&
        !std::is_same_v<std::decay_t<T>, std::string> &&
        !std::is_same_v<std::decay_t<T>, std::string_view>,
        int> = 0,
        typename = decltype(std::declval<const T&>().toUtf8().constData())>
    CFString(const T & qstr) {
        auto utf8 = qstr.toUtf8();
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

    CFByteArray toUtf8()  const { return CFByteArray(data_.data(), (cfsize_t)data_.size()); }
    CFByteArray toLatin1() const { return toUtf8(); }

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
    cfsize_t indexOf(const CFString & s, cfsize_t from = 0) const {
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
    bool startsWith(const CFString & s) const { return data_.rfind(s.data_, 0) == 0; }
    bool endsWith(const char * s) const {
        cfsize_t slen = strlen(s);
        if (slen > (cfsize_t)data_.size()) return false;
        return data_.compare(data_.size() - slen, slen, s) == 0;
    }
    bool endsWith(const CFString & s) const {
        if (s.data_.size() > data_.size()) return false;
        return data_.compare(data_.size() - s.data_.size(), s.data_.size(), s.data_) == 0;
    }

    CFString mid(cfsize_t bytePos, cfsize_t len = npos) const {
        if (bytePos >= (cfsize_t)data_.size()) return CFString();
        return CFString(data_.substr(bytePos, len));
    }
    CFString left(cfsize_t n) const { return mid(0, n); }
    CFString right(cfsize_t n) const {
        if (n >= (cfsize_t)data_.size()) return *this;
        return mid(data_.size() - n);
    }

    CFString trimmed() const {
        cfsize_t s = data_.find_first_not_of(" \t\r\n");
        if (s == std::string::npos) return CFString();
        cfsize_t e = data_.find_last_not_of(" \t\r\n");
        return CFString(data_.substr(s, e - s + 1));
    }

    CFString toLower() const {
        std::string r = data_;
        for (char & c : r) if (c >= 'A' && c <= 'Z') c += 32;
        return CFString(std::move(r));
    }
    CFString toUpper() const {
        std::string r = data_;
        for (char & c : r) if (c >= 'a' && c <= 'z') c -= 32;
        return CFString(std::move(r));
    }

    std::vector<CFString> split(char delim) const {
        std::vector<CFString> result;
        cfsize_t start = 0, pos;
        while ((pos = data_.find(delim, start)) != std::string::npos) {
            result.push_back(CFString(data_.substr(start, pos - start)));
            start = pos + 1;
        }
        result.push_back(CFString(data_.substr(start)));
        return result;
    }
    std::vector<CFString> split(const char * delim) const {
        std::vector<CFString> result;
        cfsize_t dlen = strlen(delim);
        cfsize_t start = 0, pos;
        while ((pos = data_.find(delim, start)) != std::string::npos) {
            result.push_back(CFString(data_.substr(start, pos - start)));
            start = pos + dlen;
        }
        result.push_back(CFString(data_.substr(start)));
        return result;
    }

    CFString & replace(const char * before, const char * after) {
        const cfsize_t blen = strlen(before);
        const cfsize_t alen = strlen(after);
        cfsize_t pos = 0;
        while ((pos = data_.find(before, pos)) != std::string::npos) {
            data_.replace(pos, blen, after, alen);
            pos += alen;
        }
        return *this;
    }
    CFString & replace(cfsize_t pos, cfsize_t len, const char * s) {
        data_.replace(pos, len, s);
        return *this;
    }

    CFString join(const std::vector<CFString> & list) const {
        std::string r;
        for (cfsize_t i = 0; i < (cfsize_t)list.size(); ++i) {
            if (i > 0) r += data_;
            r += list[i].data_;
        }
        return CFString(std::move(r));
    }

    static CFString number(cfint64 v) {
        return CFString(std::format("{}", (long long)v).c_str());
    }
    static CFString number(cfuint64 v) {
        return CFString(std::format("{}", (unsigned long long)v).c_str());
    }
    static CFString number(double v) {
        return CFString(std::format("{:g}", v).c_str());
    }
    static CFString number(cfint32  v) { return number((cfint64)v); }
    static CFString number(cfuint32 v) { return number((cfuint64)v); }

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

    static CFString fromUtf8(const char * s, cfsize_t len = npos) {
        if (!s) return CFString();
        return CFString(len == npos ? std::string(s) : std::string(s, len));
    }
    static CFString fromUtf8(const CFByteArray & ba) {
        return CFString(std::string(ba.constData(), ba.size()));
    }
    static CFString fromLatin1(const char * s) { return s ? CFString(s) : CFString(); }

    bool operator==(const CFString & o) const noexcept { return data_ == o.data_; }
    bool operator==(const char * o)     const noexcept { return o ? data_ == o : isNull_; }
    bool operator!=(const CFString & o) const noexcept { return data_ != o.data_; }
    bool operator!=(const char * o)     const noexcept { return !((*this) == o); }
    bool operator< (const CFString & o) const noexcept { return data_ <  o.data_; }
    bool operator<=(const CFString & o) const noexcept { return data_ <= o.data_; }
    bool operator> (const CFString & o) const noexcept { return data_ >  o.data_; }
    bool operator>=(const CFString & o) const noexcept { return data_ >= o.data_; }

    CFString   operator+ (const CFString & o) const { return CFString(data_ + o.data_); }
    CFString   operator+ (const char * s)     const { return CFString(data_ + s); }
    CFString & operator+=(const CFString & o) { data_ += o.data_; isNull_ = false; return *this; }
    CFString & operator+=(const char * s)     { data_ += s;       isNull_ = false; return *this; }
    CFString & operator+=(char c)             { data_ += c;       isNull_ = false; return *this; }

    CFString & operator<<(const CFString & o) { return *this += o; }
    CFString & operator<<(const char * s)     { return *this += s; }
    CFString & operator<<(char c)             { return *this += c; }

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

    CFString simplified() const {
        std::string r;
        bool lastWasSpace = true;
        for (char c : data_) {
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
                if (!lastWasSpace) { r += ' '; lastWasSpace = true; }
            } else { r += c; lastWasSpace = false; }
        }
        if (!r.empty() && r.back() == ' ') r.pop_back();
        return CFString(std::move(r));
    }

private:
    std::string data_;
    bool isNull_ = true;
};

inline bool operator==(const char * lhs, const CFString & rhs) { return rhs == lhs; }
inline bool operator!=(const char * lhs, const CFString & rhs) { return rhs != lhs; }

inline CFString operator+(const char * lhs, const CFString & rhs) {
    return CFString(std::string(lhs) + rhs.str());
}
// Free-function operator<< removed: member operator<< covers these cases

namespace std {
template<> struct hash<CFString> {
    size_t operator()(const CFString & s) const noexcept {
        return hash<string>()(s.str());
    }
};
}

/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/base/string.h>

#include <cstring>

namespace cflib::base {

size_t String::charCount() const {
    size_t count = 0;
    const uint8 * p = reinterpret_cast<const uint8 *>(d->data());
    for (size_t i = 0; i < d->size; ) {
        i += Shared::utf8Step(p[i]);
        ++count;
    }
    return count;
}

ssize_t String::lastIndexOf(const char * s) const {
    size_t pos = d->toStdStringView().rfind(s);
    return pos == std::string::npos ? -1 : (ssize_t)pos;
}

size_t String::count(const String & subStr) const {
    std::string_view sub = subStr.toStdStringView();
    if (sub.empty()) return 0;
    size_t rv = 0;
    size_t pos = 0;
    while ((pos = d->toStdStringView().find(sub, pos)) != std::string::npos) {
        ++rv;
        pos += sub.size();
    }
    return rv;
}

String String::mid(size_t bytePos, ssize_t len) const {
    if (bytePos >= d->size) return String();
    size_t avail = d->size - bytePos;
    return String(std::string_view(d->data() + bytePos, len == -1 || (size_t)len > avail ? avail : len));
}

String String::trimmed() const {
    std::string_view sv = d->toStdStringView();
    size_t s = sv.find_first_not_of(" \t\r\n");
    if (s == std::string::npos) return String();
    size_t e = sv.find_last_not_of(" \t\r\n");
    return String(std::string_view(sv.data() + s, e - s + 1));
}

String String::toLower() const {
    String out(d->data(), d->size);
    for (size_t i = 0; i < out.size(); ++i) {
        char & c = out[i];
        if (c >= 'A' && c <= 'Z') c += 32;
    }
    return out;
}

String String::toUpper() const {
    String out(d->data(), d->size);
    for (size_t i = 0; i < out.size(); ++i) {
        char & c = out[i];
        if (c >= 'a' && c <= 'z') c -= 32;
    }
    return out;
}

StringList String::split(char delim) const {
    StringList result;
    size_t start = 0, pos;
    std::string_view sv = d->toStdStringView();
    while ((pos = sv.find(delim, start)) != std::string::npos) {
        result << String(std::string_view(sv.data() + start, pos - start));
        start = pos + 1;
    }
    result << String(std::string_view(sv.data() + start, sv.size() - start));
    return result;
}

StringList String::split(const char * delim) const {
    size_t dlen = std::strlen(delim);
    StringList result;
    size_t start = 0, pos;
    std::string_view sv = d->toStdStringView();
    while ((pos = sv.find(delim, start)) != std::string::npos) {
        result << String(std::string_view(sv.data() + start, pos - start));
        start = pos + dlen;
    }
    result << String(std::string_view(sv.data() + start, sv.size() - start));
    return result;
}

String String::join(const StringList & list) const {
    size_t total = 0;
    for (size_t i = 0; i < list.size(); ++i) total += list[i].size();
    if (list.size() > 1) total += (list.size() - 1) * size();

    String out;
    out.reserve(total);
    for (size_t i = 0; i < list.size(); ++i) {
        if (i > 0) out += *this;
        out += list[i];
    }
    return out;
}

String String::fromUtf8(const char * s, ssize_t len) {
    if (!s) return String();
    return String(std::string_view(s, len == -1 ? std::strlen(s) : len));
}

// The bodies must not call `rhs == lhs` / `rhs != lhs`: under C++20
// rewritten-candidate rules that expression would resolve back to these
// free functions themselves (infinite recursion). Comparing the byte
// buffers directly keeps them recursion-proof and mirrors the
// null-aware member semantics of ByteArray. They are also required for
// the `char* == String` direction, which GCC 13 fails to resolve via the
// member operator alone.
bool operator==(const char * lhs, const String & rhs) {
    return lhs ? rhs.str() == std::string_view(lhs) : rhs.isNull();
}
bool operator!=(const char * lhs, const String & rhs) {
    return lhs ? !(rhs.str() == std::string_view(lhs)) : !rhs.isNull();
}

String operator+(const char * lhs, const String & rhs) {
    return String(std::string(lhs) + std::string(rhs.str()));
}

} // namespace

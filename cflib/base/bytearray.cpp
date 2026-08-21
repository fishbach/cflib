/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/base/bytearray.h>

#include <cstdlib>

namespace cflib::base {

// the buffer is not NUL-terminated, so the C parsers get a local copy
uint32 ByteArray::toUInt(bool * ok) const {
    if (d->size == 0) { if (ok) *ok = false; return 0; }
    std::string s(d->data(), d->size);
    char * end = nullptr;
    unsigned long v = std::strtoul(s.c_str(), &end, 10);
    if (ok) *ok = (end != s.c_str() && *end == '\0');
    return (uint32)v;
}

int32 ByteArray::toInt(bool * ok) const {
    if (d->size == 0) { if (ok) *ok = false; return 0; }
    std::string s(d->data(), d->size);
    char * end = nullptr;
    long v = std::strtol(s.c_str(), &end, 10);
    if (ok) *ok = (end != s.c_str() && *end == '\0');
    return (int32)v;
}

uint64 ByteArray::toULong(bool * ok) const {
    if (d->size == 0) { if (ok) *ok = false; return 0; }
    std::string s(d->data(), d->size);
    char * end = nullptr;
    unsigned long long v = std::strtoull(s.c_str(), &end, 10);
    if (ok) *ok = (end != s.c_str() && *end == '\0');
    return (uint64)v;
}

int64 ByteArray::toLong(bool * ok) const {
    if (d->size == 0) { if (ok) *ok = false; return 0; }
    std::string s(d->data(), d->size);
    char * end = nullptr;
    long long v = std::strtoll(s.c_str(), &end, 10);
    if (ok) *ok = (end != s.c_str() && *end == '\0');
    return (int64)v;
}

ByteArray & ByteArray::replace(const char * before, const char * after) {
    const size_t blen = std::strlen(before);
    const size_t alen = std::strlen(after);
    if (blen == 0) return *this;
    detach();
    size_t pos = 0;
    std::string_view hay = d->toStdStringView();
    while ((pos = hay.find(before, pos)) != std::string::npos) {
        d = d->replace(pos, blen, after, alen);
        hay = d->toStdStringView();
        pos += alen;
    }
    return *this;
}

ByteArray & ByteArray::replace(char before, const char * after) {
    const size_t alen = std::strlen(after);
    detach();
    size_t pos = 0;
    std::string_view hay = d->toStdStringView();
    while ((pos = hay.find(before, pos)) != std::string::npos) {
        d = d->replace(pos, 1, after, alen);
        hay = d->toStdStringView();
        pos += alen;
    }
    return *this;
}

ByteArray ByteArray::trimmed() const {
    std::string_view sv = d->toStdStringView();
    size_t s = sv.find_first_not_of(" \t\r\n");
    if (s == std::string::npos) return ByteArray();
    size_t e = sv.find_last_not_of(" \t\r\n");
    return ByteArray(sv.data() + s, e - s + 1);
}

// trim and collapse runs of whitespace to single spaces
ByteArray ByteArray::simplified() const {
    ByteArray out;
    out.reserve(d->size);
    bool lastWasSpace = true;
    for (size_t i = 0; i < d->size; ++i) {
        char c = d->data()[i];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            if (!lastWasSpace) { out.append(' '); lastWasSpace = true; }
        } else { out.append(c); lastWasSpace = false; }
    }
    if (out.size() && out.at(out.size() - 1) == ' ') out.remove(out.size() - 1, 1);
    return out;
}

ByteArray ByteArray::toLower() const {
    ByteArray out(d->data(), d->size);
    for (size_t i = 0; i < out.size(); ++i) {
        char & c = out[i];
        if (c >= 'A' && c <= 'Z') c += 32;
    }
    return out;
}

ByteArrayList ByteArray::split(char delim) const
{
    std::vector<ByteArray> result;
    size_t start = 0, pos;
    std::string_view sv = d->toStdStringView();
    while ((pos = sv.find(delim, start)) != std::string::npos) {
        result.push_back(ByteArray(sv.data() + start, pos - start));
        start = pos + 1;
    }
    result.push_back(ByteArray(sv.data() + start, sv.size() - start));
    return result;
}

ByteArray ByteArray::toBase64() const {
    static const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    out.reserve(((d->size + 2) / 3) * 4);
    size_t i = 0;
    const char * p = d->data();
    while (i + 2 < d->size) {
        uint32 n = ((uint8)p[i] << 16) | ((uint8)p[i+1] << 8) | (uint8)p[i+2];
        out += table[(n >> 18) & 0x3F];
        out += table[(n >> 12) & 0x3F];
        out += table[(n >>  6) & 0x3F];
        out += table[n & 0x3F];
        i += 3;
    }
    if (i + 1 == d->size) {
        uint32 n = (uint8)p[i] << 16;
        out += table[(n >> 18) & 0x3F];
        out += table[(n >> 12) & 0x3F];
        out += '=';
        out += '=';
    } else if (i + 2 == d->size) {
        uint32 n = ((uint8)p[i] << 16) | ((uint8)p[i+1] << 8);
        out += table[(n >> 18) & 0x3F];
        out += table[(n >> 12) & 0x3F];
        out += table[(n >>  6) & 0x3F];
        out += '=';
    }
    return ByteArray(std::move(out));
}

ByteArray ByteArray::fromBase64(const ByteArray & base64) {
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

ByteArray ByteArray::fromHex(const char * hex) {
    ByteArray out;
    size_t len = std::strlen(hex);
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
        out.append((char)((hi << 4) | lo));
        i += 2;
    }
    return out;
}

ByteArray ByteArray::toHex() const {
    static const char * digits = "0123456789abcdef";
    ByteArray out(d->size * 2, '\0');
    for (size_t i = 0; i < d->size; ++i) {
        uint8 c = (uint8)d->data()[i];
        out[i*2]   = digits[c >> 4];
        out[i*2+1] = digits[c & 0xF];
    }
    return out;
}

} // namespace

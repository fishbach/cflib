/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base/concurrent.h>
#include <cflib/base/container.h>

#include <concepts>
#include <cstdlib>
#include <cstring>
#include <format>
#include <new>
#include <string>
#include <string_view>
#include <vector>

namespace cflib::base {

// The shared payload of a ByteArray: one heap block per unique content.
//
// The block consists of this fixed 32 byte header (refcount, size,
// capacity, flags) immediately followed by the byte buffer, so bytes and
// bookkeeping live in the same cache region. Strings up to
// InlineCapacity occupy exactly one BlockSize block; longer strings use
// one block of HeaderSize + capacity.
//
// Growth: capacity is a power of two, minimum CapacityMin. Growing to
// `need` uses the smallest such capacity (realloc, which may extend in
// place). Shrinking never reduces capacity.
struct ByteArrayShared
{
    static constexpr size_t HeaderSize     = 32;
    static constexpr size_t InlineCapacity = 256;
    static constexpr size_t BlockSize      = HeaderSize + InlineCapacity;
    static constexpr size_t CapacityMin    = 256;

    AtomicInt  ref{1};
    size_t     size{0};
    size_t     capacity{0};
    uint32_t   flags{0};

    ByteArrayShared() = default;
    ~ByteArrayShared() = default;
    ByteArrayShared(size_t s, size_t c, uint32_t f = 0) : size(s), capacity(c), flags(f) {}

    bool isNull() const { return flags & 1u; }
    void setNull(bool v) { v ? (flags |= 1u) : (flags &= ~1u); }

    char *       data()       { return reinterpret_cast<char *>(this) + HeaderSize; }
    const char * data() const { return reinterpret_cast<const char *>(this) + HeaderSize; }
    std::string_view sv() const { return std::string_view(data(), size); }

    // smallest power of two that is >= max(need, CapacityMin)
    static size_t blockCapacity(size_t need)
    {
        size_t cap = CapacityMin;
        while (cap < need) cap <<= 1;
        return cap;
    }

    // fresh block owning a copy of [p, p+len)
    static ByteArrayShared * create(const char * p, size_t len)
    {
        size_t cap = blockCapacity(len);
        void * mem = std::malloc(HeaderSize + cap);
        if (!mem) std::terminate();
        ByteArrayShared * s = new (mem) ByteArrayShared(len, cap);
        if (len) std::memcpy(s->data(), p, len);
        return s;
    }
    // fresh block of n bytes, all set to c
    static ByteArrayShared * create(size_t n, char c)
    {
        size_t cap = blockCapacity(n);
        void * mem = std::malloc(HeaderSize + cap);
        if (!mem) std::terminate();
        ByteArrayShared * s = new (mem) ByteArrayShared(n, cap);
        if (n) std::memset(s->data(), c, n);
        return s;
    }
    // exclusive copy of an existing block (same capacity)
    static ByteArrayShared * copy(const ByteArrayShared & o)
    {
        size_t cap = blockCapacity(o.capacity ? o.capacity : CapacityMin);
        void * mem = std::malloc(HeaderSize + cap);
        if (!mem) std::terminate();
        ByteArrayShared * s = new (mem) ByteArrayShared(o.size, cap, o.flags);
        if (o.size) std::memcpy(s->data(), o.data(), o.size);
        return s;
    }

    // grow the block so it can hold `need` bytes; realloc may move it,
    // so callers must use the returned pointer
    ByteArrayShared * grow(size_t need)
    {
        if (need <= capacity) return this;
        size_t cap = blockCapacity(need);
        void * mem = std::realloc(static_cast<void *>(this), HeaderSize + cap);
        if (!mem) std::terminate();
        ByteArrayShared * m = static_cast<ByteArrayShared *>(mem);
        m->capacity = cap;
        return m;
    }
    ByteArrayShared * append(const char * p, size_t len)
    {
        ByteArrayShared * s = grow(size + len);
        std::memcpy(s->data() + s->size, p, len);
        s->size += len;
        return s;
    }
    // positions are clamped to [0, size]; callers may pass npos-style values
    ByteArrayShared * insert(size_t pos, const char * p, size_t len)
    {
        if (pos > size) pos = size;
        ByteArrayShared * s = grow(size + len);
        std::memmove(s->data() + pos + len, s->data() + pos, s->size - pos);
        std::memcpy(s->data() + pos, p, len);
        s->size += len;
        return s;
    }
    // erase [pos, pos+len), clamped to [0, size]; no reallocation
    void remove(size_t pos, size_t len)
    {
        if (pos >= size) return;
        if (pos + len > size) len = size - pos;
        std::memmove(data() + pos, data() + pos + len, size - pos - len);
        size -= len;
    }
    ByteArrayShared * replace(size_t pos, size_t len, const char * p, size_t plen)
    {
        if (pos + len > size) {
            // past the end: NUL-pad to pos, then append
            ByteArrayShared * s = grow(pos + plen);
            if (pos > s->size) std::memset(s->data() + s->size, 0, pos - s->size);
            std::memcpy(s->data() + pos, p, plen);
            s->size = pos + plen;
            return s;
        }
        ByteArrayShared * s = grow(size + plen - len);
        std::memmove(s->data() + pos + plen, s->data() + pos + len, s->size - pos - len);
        std::memcpy(s->data() + pos, p, plen);
        s->size += plen - len;
        return s;
    }

    // UTF-8 step width (used by String::charCount)
    static size_t utf8Step(uint8 c) {
        if      (c < 0x80) return 1;
        else if (c < 0xE0) return 2;
        else if (c < 0xF0) return 3;
        else               return 4;
    }
};

// process-wide null payload: default-constructed ByteArrays and copies of
// null values share it, so they cost zero allocations. All mutation
// paths go through detach(), which never writes to it; it is never
// deleted.
inline ByteArrayShared * byteArrayNullShared()
{
    static ByteArrayShared s(0, 0, 1u); // isNull flag
    return &s;
}

class ByteArray
{
public:
    using Shared = ByteArrayShared;
    static constexpr size_t npos = std::string::npos;

    ByteArray() : d(byteArrayNullShared()) {}
    ByteArray(const char * data) : d(data ? Shared::create(data, std::strlen(data)) : byteArrayNullShared()) {}
    ByteArray(const char * data, size_t len) : d(Shared::create(data, len)) {}
    ByteArray(const uint8 * data, size_t len) : d(Shared::create(reinterpret_cast<const char *>(data), len)) {}
    ByteArray(size_t n, char c) : d(Shared::create(n, c)) {}
    ByteArray(std::string_view sv) : d(Shared::create(sv.data(), sv.size())) {}
    explicit ByteArray(std::string s) : d(Shared::create(s.data(), s.size())) {}

    // implicit sharing
    ByteArray(const ByteArray & other) : d(other.d) { if (d != byteArrayNullShared()) d->ref.ref(); }
    ByteArray(ByteArray && other) : d(other.d) { other.d = byteArrayNullShared(); }
    ~ByteArray() { release(); }

    ByteArray & operator=(const ByteArray & other) {
        if (d == other.d) return *this;
        release();
        d = other.d;
        if (d != byteArrayNullShared()) d->ref.ref();
        return *this;
    }
    ByteArray & operator=(ByteArray && other) {
        if (this == &other) return *this;
        release();                     // drop the block we currently hold
        d = other.d;                   // take over other's block
        other.d = byteArrayNullShared();
        return *this;
    }
    // acquire an exclusive block when the content is shared
    void detach() {
        if (d != byteArrayNullShared() && d->ref.loadAcquire() == 1) return;
        Shared * s = Shared::copy(*d);
        if (d != byteArrayNullShared() && !d->ref.deref()) deleteShared(d);
        d = s;
    }

    static ByteArray fromRawData(const char * data, size_t len) { return ByteArray(data, len); }

    char *       data()       { detach(); return d->data(); }
    const char * data() const { return d->data(); }
    const char * constData() const { return d->data(); }
    std::string_view sv() const { return d->sv(); }

    size_t size()   const { return d->size; }
    size_t length() const { return d->size; }
    bool     isEmpty() const { return d->size == 0; }
    bool     isNull() const { return d->isNull(); }

    void resize(size_t n) {
        detach();
        d = d->grow(n);
        d->size = n;
        d->setNull(false);
    }
    void resize(size_t n, char c) {
        detach();
        d = d->grow(n);
        if (n > d->size) std::memset(d->data() + d->size, c, n - d->size);
        d->size = n;
        d->setNull(false);
    }
    void reserve(size_t n) { detach(); d = d->grow(n); }
    void clear() { detach(); d->size = 0; d->setNull(true); }
    size_t capacity() const { return d->capacity; }

    char   operator[](size_t i) const { return d->data()[i]; }
    char & operator[](size_t i)       { detach(); return d->data()[i]; }
    char   at(size_t i) const         { return d->data()[i]; }

    ByteArray & append(char c)                    { detach(); d = d->append(&c, 1); d->setNull(false); return *this; }
    ByteArray & append(const char * s)            { if (s && *s) { detach(); d = d->append(s, std::strlen(s)); d->setNull(false); } return *this; }
    ByteArray & append(const char * s, size_t len){ if (s && len) { detach(); d = d->append(s, len); d->setNull(false); } return *this; }
    ByteArray & append(const ByteArray & other)   { return append(other.constData(), other.size()); }

    ByteArray & prepend(const char * s, size_t len) {
        if (!s || !len) return *this;
        detach();
        d = d->insert(0, s, len);
        d->setNull(false);
        return *this;
    }
    ByteArray & prepend(const char * s) { return prepend(s, std::strlen(s)); }

    ByteArray & insert(size_t pos, const ByteArray & ba) { return insert(pos, ba.constData(), ba.size()); }
    ByteArray & insert(size_t pos, const char * s, size_t len) {
        if (!s || !len) return *this;
        detach();
        d = d->insert(pos, s, len);
        d->setNull(false);
        return *this;
    }

    ByteArray mid(size_t pos, size_t len = npos) const {
        if (pos >= d->size) return ByteArray();
        size_t avail = d->size - pos;
        return ByteArray(d->data() + pos, len == npos || len > avail ? avail : len);
    }
    ByteArray left(size_t n) const  { return mid(0, n); }
    ByteArray right(size_t n) const {
        if (n >= d->size) return *this;
        return mid(d->size - n);
    }

    bool startsWith(const char * s) const { return d->sv().starts_with(s); }
    bool startsWith(const ByteArray & other) const { return d->sv().starts_with(other.d->sv()); }
    bool endsWith(const char * s) const {
        std::string_view sv = d->sv();
        size_t slen = std::char_traits<char>::length(s);
        if (slen > sv.size()) return false;
        return sv.compare(sv.size() - slen, slen, s) == 0;
    }
    bool endsWith(const ByteArray & other) const { return d->sv().ends_with(other.d->sv()); }
    bool contains(const char * s) const { return d->sv().find(s) != npos; }
    bool contains(char c) const { return d->sv().find(c) != npos; }
    bool contains(const ByteArray & other) const { return d->sv().find(other.d->sv()) != npos; }

    ssize_t indexOf(char c, size_t from = 0) const {
        size_t pos = d->sv().find(c, from);
        return pos == npos ? -1 : (ssize_t)pos;
    }
    ssize_t indexOf(const char * s, size_t from = 0) const {
        size_t pos = d->sv().find(s, from);
        return pos == npos ? -1 : (ssize_t)pos;
    }
    ssize_t indexOf(const ByteArray & other, size_t from = 0) const {
        size_t pos = d->sv().find(other.d->sv(), from);
        return pos == npos ? -1 : (ssize_t)pos;
    }

    // replace(pos, len, newData, newLen) -- in-place substitution
    ByteArray & replace(size_t pos, size_t len, const char * newData, size_t newLen) {
        detach();
        d = d->replace(pos, len, newData ? newData : "", newLen);
        return *this;
    }
    ByteArray & replace(size_t pos, size_t len, const char * newData) {
        detach();
        d = d->replace(pos, len, newData ? newData : "", newData ? std::strlen(newData) : 0);
        return *this;
    }

    // replace all occurrences of `before` with `after`
    ByteArray & replace(const char * before, const char * after);
    // replace all occurrences of a single byte
    ByteArray & replace(char before, const char * after);

    // Numeric conversions (implemented in bytearray.cpp)
    uint32 toUInt(bool * ok = nullptr) const;
    int32  toInt(bool * ok = nullptr) const;
    uint64 toULong(bool * ok = nullptr) const;
    int64  toLong(bool * ok = nullptr) const;

    ByteArray trimmed() const;
    // trim and collapse runs of whitespace to single spaces
    ByteArray simplified() const;
    ByteArray toLower() const;

    // split by character
    std::vector<ByteArray> split(char delim) const;

    // remove bytes at position
    ByteArray & remove(size_t pos, size_t len) {
        detach();
        d->remove(pos, len);
        return *this;
    }

    // Base64 encoding/decoding (implemented in bytearray.cpp)
    ByteArray toBase64() const;
    static ByteArray fromBase64(const ByteArray & base64);

    // explicit, owning conversion: copies the bytes (O(n))
    std::string toStdString() const { return std::string(d->data(), d->size); }
    explicit operator std::string() const { return toStdString(); }

    // Hex conversion (implemented in bytearray.cpp)
    static ByteArray fromHex(const char * hex);
    static ByteArray fromHex(const ByteArray & hex) { return fromHex(hex.toStdString().c_str()); }
    ByteArray toHex() const;

    // Number formatting
    static ByteArray number(std::integral auto v) {
        return ByteArray(std::format("{}", v).c_str());
    }
    static ByteArray number(std::floating_point auto v) {
        return ByteArray(std::format("{:g}", v).c_str());
    }

    ByteArray & operator+=(char c)              { return append(c); }
    ByteArray & operator+=(const char * s)      { return append(s); }
    ByteArray & operator+=(const ByteArray & o) { return append(o); }

    ByteArray operator+(char c)              const { ByteArray r(*this); r += c; return r; }
    ByteArray operator+(const char * s)      const { ByteArray r(*this); r += s; return r; }
    ByteArray operator+(const ByteArray & o) const { ByteArray r(*this); r += o; return r; }

    bool operator==(const ByteArray & o) const { return d->sv() == o.d->sv(); }
    bool operator!=(const ByteArray & o) const { return d->sv() != o.d->sv(); }
    bool operator< (const ByteArray & o) const { return d->sv() <  o.d->sv(); }
    // compare against an explicit string_view: `sv == const char*` is
    // ambiguous under C++20 rewritten candidates (GCC 13, fixed in C++23)
    bool operator==(const char * o) const { return o ? d->sv() == std::string_view(o) : d->isNull(); }
    bool operator!=(const char * o) const { return !((*this) == o); }

protected:
    Shared * d;
    // raw byte append, for subclasses that manage the null flag themselves
    void appendBytes(const char * s, size_t len) {
        if (!s || !len) return;
        detach();
        d = d->append(s, len);
        d->setNull(false);
    }

private:
    static void deleteShared(const Shared * s) {
        s->~ByteArrayShared();
        std::free(const_cast<void *>(static_cast<const void *>(s)));
    }
    void release() {
        if (d != byteArrayNullShared() && !d->ref.deref()) deleteShared(d);
    }
};

inline ByteArray operator+(const char * lhs, const ByteArray & rhs) {
    ByteArray r(lhs);
    r += rhs;
    return r;
}

// The bodies must not call `rhs == lhs` / `rhs != lhs`: under C++20
// rewritten-candidate rules that expression would resolve back to these
// free functions themselves (infinite recursion). Comparing the byte
// buffers directly keeps them recursion-proof and mirrors the
// null-aware member semantics. They are also required for the `char* ==
// ByteArray` direction, which GCC 13 fails to resolve via the member
// operator alone (see string.h).
inline bool operator==(const char * lhs, const ByteArray & rhs) {
    return lhs ? rhs.sv() == std::string_view(lhs) : rhs.isNull();
}
inline bool operator!=(const char * lhs, const ByteArray & rhs) {
    return lhs ? !(rhs.sv() == std::string_view(lhs)) : !rhs.isNull();
}

inline ByteArray & operator<<(ByteArray & lhs, const ByteArray & rhs) { return lhs += rhs; }
inline ByteArray & operator<<(ByteArray & lhs, const char * rhs)      { return lhs += rhs; }
inline ByteArray & operator<<(ByteArray & lhs, char rhs)              { return lhs += rhs; }

using ByteArrayList = List<ByteArray>;

} // namespace

namespace std {
template<> struct hash<cflib::base::ByteArray> {
    size_t operator()(const cflib::base::ByteArray & ba) const {
        return hash<string_view>()(ba.sv());
    }
};
}

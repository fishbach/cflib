/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base/bytearray.h>
#include <cflib/base/concurrent.h>

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
// bookkeeping live in the same cache region.
//
// Growth: capacity is a power of two, minimum CapacityMin. Growing to
// `need` uses the smallest such capacity (realloc, which may extend in
// place). Shrinking never reduces capacity.
struct ByteArrayShared
{
    static constexpr size_t HeaderSize  = 32;
    static constexpr size_t CapacityMin = 32;

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
    // exclusive copy of an existing block (same capacity), unless `hint`
    // demands a larger one, so a resize/reserve right after the copy
    // does not need a second allocation
    static ByteArrayShared * copy(const ByteArrayShared & o, size_t hint = 0)
    {
        size_t cap = blockCapacity(o.capacity ? o.capacity : CapacityMin);
        if (hint > cap) cap = blockCapacity(hint);
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
    // grow() may realloc the block to a new address, freeing the old one. If the
    // source range [p, p+len) overlaps this block's live buffer, reading it after
    // grow() would be a use-after-free (e.g. self-append `ba += ba`). Snapshot the
    // bytes into `tmp` and return a safe pointer; otherwise return p unchanged.
    const char * safeSource(const char * p, size_t len, std::vector<char> & tmp) const
    {
        if (p >= data() && p < data() + size) {
            tmp.assign(p, p + len);
            return tmp.data();
        }
        return p;
    }

    ByteArrayShared * append(const char * p, size_t len)
    {
        std::vector<char> tmp;
        const char * src = safeSource(p, len, tmp);
        ByteArrayShared * s = grow(size + len);
        std::memcpy(s->data() + s->size, src, len);
        s->size += len;
        return s;
    }
    // positions are clamped to [0, size]; callers may pass npos-style values
    ByteArrayShared * insert(size_t pos, const char * p, size_t len)
    {
        if (pos > size) pos = size;
        std::vector<char> tmp;
        const char * src = safeSource(p, len, tmp);
        ByteArrayShared * s = grow(size + len);
        std::memmove(s->data() + pos + len, s->data() + pos, s->size - pos);
        std::memcpy(s->data() + pos, src, len);
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
        std::vector<char> tmp;
        const char * src = safeSource(p, plen, tmp);
        if (pos + len > size) {
            // past the end: NUL-pad to pos, then append
            ByteArrayShared * s = grow(pos + plen);
            if (pos > s->size) std::memset(s->data() + s->size, 0, pos - s->size);
            std::memcpy(s->data() + pos, src, plen);
            s->size = pos + plen;
            return s;
        }
        ByteArrayShared * s = grow(size + plen - len);
        std::memmove(s->data() + pos + plen, s->data() + pos + len, s->size - pos - len);
        std::memcpy(s->data() + pos, src, plen);
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

inline ByteArray::ByteArray() : d(byteArrayNullShared()) {}
inline ByteArray::ByteArray(const char * data) : d(data ? Shared::create(data, std::strlen(data)) : byteArrayNullShared()) {}
inline ByteArray::ByteArray(const char * data, size_t len) : d(Shared::create(data, len)) {}
inline ByteArray::ByteArray(const uint8 * data, size_t len) : d(Shared::create(reinterpret_cast<const char *>(data), len)) {}
inline ByteArray::ByteArray(size_t n, char c) : d(Shared::create(n, c)) {}
inline ByteArray::ByteArray(std::string_view sv) : d(Shared::create(sv.data(), sv.size())) {}
inline ByteArray::ByteArray(std::string s) : d(Shared::create(s.data(), s.size())) {}

// implicit sharing
inline ByteArray::ByteArray(const ByteArray & other) : d(other.d) { if (d != byteArrayNullShared()) d->ref.ref(); }
inline ByteArray::ByteArray(ByteArray && other) : d(other.d) { other.d = byteArrayNullShared(); }
inline ByteArray::~ByteArray() { release(); }

inline ByteArray & ByteArray::operator=(const ByteArray & other) {
    if (d == other.d) return *this;
    release();
    d = other.d;
    if (d != byteArrayNullShared()) d->ref.ref();
    return *this;
}
inline ByteArray & ByteArray::operator=(ByteArray && other) {
    if (this == &other) return *this;
    release();                     // drop the block we currently hold
    d = other.d;                   // take over other's block
    other.d = byteArrayNullShared();
    return *this;
}
// acquire an exclusive block when the content is shared; when this
// detaches, the fresh block is allocated with at least blockCapacity(hint)
inline void ByteArray::detach(size_t hint) {
    if (d != byteArrayNullShared() && d->ref.loadAcquire() == 1) return;
    Shared * s = Shared::copy(*d, hint);
    if (d != byteArrayNullShared() && !d->ref.deref()) deleteShared(d);
    d = s;
}

inline ByteArray ByteArray::fromRawData(const char * data, size_t len) { return ByteArray(data, len); }

inline char * ByteArray::data()       { detach(); return d->data(); }
inline const char * ByteArray::data() const { return d->data(); }
inline const char * ByteArray::constData() const { return d->data(); }
inline std::string_view ByteArray::sv() const { return d->sv(); }

inline size_t ByteArray::size()   const { return d->size; }
inline size_t ByteArray::length() const { return d->size; }
inline bool     ByteArray::isEmpty() const { return d->size == 0; }
inline bool     ByteArray::isNull() const { return d->isNull(); }

inline void ByteArray::resize(size_t n) {
    detach(n);
    d = d->grow(n);
    d->size = n;
    d->setNull(false);
}
inline void ByteArray::resize(size_t n, char c) {
    detach(n);
    d = d->grow(n);
    if (n > d->size) std::memset(d->data() + d->size, c, n - d->size);
    d->size = n;
    d->setNull(false);
}
inline void ByteArray::reserve(size_t n) { detach(n); d = d->grow(n); }
inline void ByteArray::clear() { detach(); d->size = 0; d->setNull(true); }
inline size_t ByteArray::capacity() const { return d->capacity; }

inline char   ByteArray::operator[](size_t i) const { return d->data()[i]; }
inline char & ByteArray::operator[](size_t i)       { detach(); return d->data()[i]; }
inline char   ByteArray::at(size_t i) const         { return d->data()[i]; }

inline ByteArray & ByteArray::append(char c)                    { detach(); d = d->append(&c, 1); d->setNull(false); return *this; }
inline ByteArray & ByteArray::append(const char * s)            { if (s && *s) { detach(); d = d->append(s, std::strlen(s)); d->setNull(false); } return *this; }
inline ByteArray & ByteArray::append(const char * s, size_t len){ if (s && len) { detach(); d = d->append(s, len); d->setNull(false); } return *this; }
inline ByteArray & ByteArray::append(const ByteArray & other)   { return append(other.constData(), other.size()); }

inline ByteArray & ByteArray::prepend(const char * s, size_t len) {
    if (!s || !len) return *this;
    detach();
    d = d->insert(0, s, len);
    d->setNull(false);
    return *this;
}
inline ByteArray & ByteArray::prepend(const char * s) { return prepend(s, std::strlen(s)); }

inline ByteArray & ByteArray::insert(size_t pos, const ByteArray & ba) { return insert(pos, ba.constData(), ba.size()); }
inline ByteArray & ByteArray::insert(size_t pos, const char * s, size_t len) {
    if (!s || !len) return *this;
    detach();
    d = d->insert(pos, s, len);
    d->setNull(false);
    return *this;
}

inline ByteArray ByteArray::mid(size_t pos, size_t len) const {
    if (pos >= d->size) return ByteArray();
    size_t avail = d->size - pos;
    return ByteArray(d->data() + pos, len == npos || len > avail ? avail : len);
}
inline ByteArray ByteArray::left(size_t n) const  { return mid(0, n); }
inline ByteArray ByteArray::right(size_t n) const {
    if (n >= d->size) return *this;
    return mid(d->size - n);
}

inline bool ByteArray::startsWith(const char * s) const { return d->sv().starts_with(s); }
inline bool ByteArray::startsWith(const ByteArray & other) const { return d->sv().starts_with(other.d->sv()); }
inline bool ByteArray::endsWith(const char * s) const {
    std::string_view sv = d->sv();
    size_t slen = std::char_traits<char>::length(s);
    if (slen > sv.size()) return false;
    return sv.compare(sv.size() - slen, slen, s) == 0;
}
inline bool ByteArray::endsWith(const ByteArray & other) const { return d->sv().ends_with(other.d->sv()); }
inline bool ByteArray::contains(const char * s) const { return d->sv().find(s) != npos; }
inline bool ByteArray::contains(char c) const { return d->sv().find(c) != npos; }
inline bool ByteArray::contains(const ByteArray & other) const { return d->sv().find(other.d->sv()) != npos; }

inline ssize_t ByteArray::indexOf(char c, size_t from) const {
    size_t pos = d->sv().find(c, from);
    return pos == npos ? -1 : (ssize_t)pos;
}
inline ssize_t ByteArray::indexOf(const char * s, size_t from) const {
    size_t pos = d->sv().find(s, from);
    return pos == npos ? -1 : (ssize_t)pos;
}
inline ssize_t ByteArray::indexOf(const ByteArray & other, size_t from) const {
    size_t pos = d->sv().find(other.d->sv(), from);
    return pos == npos ? -1 : (ssize_t)pos;
}

// replace(pos, len, newData, newLen) -- in-place substitution
inline ByteArray & ByteArray::replace(size_t pos, size_t len, const char * newData, size_t newLen) {
    detach();
    d = d->replace(pos, len, newData ? newData : "", newLen);
    return *this;
}
inline ByteArray & ByteArray::replace(size_t pos, size_t len, const char * newData) {
    detach();
    d = d->replace(pos, len, newData ? newData : "", newData ? std::strlen(newData) : 0);
    return *this;
}

// remove bytes at position
inline ByteArray & ByteArray::remove(size_t pos, size_t len) {
    detach();
    d->remove(pos, len);
    return *this;
}

// explicit, owning conversion: copies the bytes (O(n))
inline std::string ByteArray::toStdString() const { return std::string(d->data(), d->size); }
inline ByteArray::operator std::string() const { return toStdString(); }

// Hex conversion (fromHex(const char *) is implemented in bytearray.cpp)
inline ByteArray ByteArray::fromHex(const ByteArray & hex) { return fromHex(hex.toStdString().c_str()); }

// Number formatting
template <std::integral T>
ByteArray ByteArray::number(T v)
{
    return ByteArray(std::format("{}", v).c_str());
}
template <std::floating_point T>
ByteArray ByteArray::number(T v)
{
    return ByteArray(std::format("{:g}", v).c_str());
}

inline ByteArray & ByteArray::operator+=(char c)              { return append(c); }
inline ByteArray & ByteArray::operator+=(const char * s)      { return append(s); }
inline ByteArray & ByteArray::operator+=(const ByteArray & o) { return append(o); }

inline ByteArray ByteArray::operator+(char c)              const { ByteArray r(*this); r += c; return r; }
inline ByteArray ByteArray::operator+(const char * s)      const { ByteArray r(*this); r += s; return r; }
inline ByteArray ByteArray::operator+(const ByteArray & o) const { ByteArray r(*this); r += o; return r; }

inline bool ByteArray::operator==(const ByteArray & o) const { return d->sv() == o.d->sv(); }
inline bool ByteArray::operator!=(const ByteArray & o) const { return d->sv() != o.d->sv(); }
inline bool ByteArray::operator< (const ByteArray & o) const { return d->sv() <  o.d->sv(); }
// compare against an explicit string_view: `sv == const char*` is
// ambiguous under C++20 rewritten candidates (GCC 13, fixed in C++23)
inline bool ByteArray::operator==(const char * o) const { return o ? d->sv() == std::string_view(o) : d->isNull(); }
inline bool ByteArray::operator!=(const char * o) const { return !((*this) == o); }

// raw byte append, for subclasses that manage the null flag themselves
inline void ByteArray::appendBytes(const char * s, size_t len) {
    if (!s || !len) return;
    detach();
    d = d->append(s, len);
    d->setNull(false);
}

inline void ByteArray::deleteShared(const Shared * s) {
    s->~ByteArrayShared();
    std::free(const_cast<void *>(static_cast<const void *>(s)));
}
inline void ByteArray::release() {
    if (d != byteArrayNullShared() && !d->ref.deref()) deleteShared(d);
}

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

} // namespace

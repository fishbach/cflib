/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base/container.h>
#include <cflib/base/types.h>

#include <concepts>
#include <string>
#include <string_view>
#include <vector>

namespace cflib::base {

// copy-on-write payload shared by ByteArray references; defined in
// impl/bytearrayimpl.h, which this header includes at the bottom
struct ByteArrayShared;

// A ByteArray is a copy-on-write sequence of raw bytes (no encoding
// semantics; for UTF-8 text use String). References to the same content
// share one heap block until one of them is mutated.
//
// Building a large string: size the buffer up front and write into it,
// rather than appending byte by byte (each append re-checks
// exclusivity and may grow the block):
//
//     ByteArray ba;
//     ba.resize(total);                    // single allocation
//     std::memcpy(ba.data(), src, total);  // direct, unchecked writes
//
// The buffer is always NUL-terminated: charPtr() is a free, NUL-terminated
// const char * (nullptr for a null value), so converting to a C string costs
// nothing. data() exposes the same buffer as a uint8 * byte view.
class ByteArray
{
public:
    using Shared = ByteArrayShared;
    static constexpr size_t npos = std::string::npos;

    ByteArray();
    ByteArray(const char * data);
    ByteArray(const char * data, size_t len);
    ByteArray(const uint8 * data, size_t len);
    ByteArray(size_t n, char c);
    ByteArray(std::string_view sv);
    explicit ByteArray(std::string s);

    // implicit sharing
    ByteArray(const ByteArray & other);
    ByteArray(ByteArray && other);
    ~ByteArray();

    ByteArray & operator=(const ByteArray & other);
    ByteArray & operator=(ByteArray && other);
    // acquire an exclusive block when the content is shared; when this
    // detaches, the fresh block is allocated with enough capacity for
    // `hint` bytes, so a resize/reserve to `hint` costs one allocation
    void detach(size_t hint = 0);

    static ByteArray fromRawData(const char * data, size_t len);

    // NUL-terminated C-string view of the buffer; nullptr for a null value.
    // Converting to a C string is therefore free (no copy). The non-const
    // overload detaches, so the bytes may then be modified in place.
    char *       charPtr();
    const char * charPtr() const;
    // raw byte view of the same buffer; nullptr for a null value
    uint8 *      data();
    const uint8 * data() const;
    const char * constData() const;
    std::string_view sv() const;

    size_t size()   const;
    size_t length() const;
    bool     isEmpty() const;
    bool     isNull() const;

    void resize(size_t n);
    void resize(size_t n, char c);
    void reserve(size_t n);
    void clear();
    size_t capacity() const;

    char   operator[](size_t i) const;
    char & operator[](size_t i);
    char   at(size_t i) const;

    ByteArray & append(char c);
    ByteArray & append(const char * s);
    ByteArray & append(const char * s, size_t len);
    ByteArray & append(const ByteArray & other);

    ByteArray & prepend(const char * s, size_t len);
    ByteArray & prepend(const char * s);

    ByteArray & insert(size_t pos, const ByteArray & ba);
    ByteArray & insert(size_t pos, const char * s, size_t len);

    ByteArray mid(size_t pos, size_t len = npos) const;
    ByteArray left(size_t n) const;
    ByteArray right(size_t n) const;

    bool startsWith(const char * s) const;
    bool startsWith(const ByteArray & other) const;
    bool endsWith(const char * s) const;
    bool endsWith(const ByteArray & other) const;
    bool contains(const char * s) const;
    bool contains(char c) const;
    bool contains(const ByteArray & other) const;

    ssize_t indexOf(char c, size_t from = 0) const;
    ssize_t indexOf(const char * s, size_t from = 0) const;
    ssize_t indexOf(const ByteArray & other, size_t from = 0) const;

    // replace(pos, len, newData, newLen) -- in-place substitution
    ByteArray & replace(size_t pos, size_t len, const char * newData, size_t newLen);
    ByteArray & replace(size_t pos, size_t len, const char * newData);

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
    ByteArray & remove(size_t pos, size_t len);

    // Base64 encoding/decoding (implemented in bytearray.cpp)
    ByteArray toBase64() const;
    static ByteArray fromBase64(const ByteArray & base64);

    // explicit, owning conversion: copies the bytes (O(n))
    std::string toStdString() const;
    explicit operator std::string() const;

    // Hex conversion (implemented in bytearray.cpp)
    static ByteArray fromHex(const char * hex);
    static ByteArray fromHex(const ByteArray & hex);
    ByteArray toHex() const;

    // Number formatting
    template <std::integral T>       static ByteArray number(T v);
    template <std::floating_point T> static ByteArray number(T v);

    ByteArray & operator+=(char c);
    ByteArray & operator+=(const char * s);
    ByteArray & operator+=(const ByteArray & o);

    ByteArray operator+(char c)              const;
    ByteArray operator+(const char * s)      const;
    ByteArray operator+(const ByteArray & o) const;

    bool operator==(const ByteArray & o) const;
    bool operator!=(const ByteArray & o) const;
    bool operator< (const ByteArray & o) const;
    // compare against an explicit string_view: `sv == const char*` is
    // ambiguous under C++20 rewritten candidates (GCC 13, fixed in C++23)
    bool operator==(const char * o) const;
    bool operator!=(const char * o) const;

protected:
    Shared * d;
    // raw byte append, for subclasses that manage the null flag themselves
    void appendBytes(const char * s, size_t len);

private:
    static void deleteShared(const Shared * s);
    void release();
};

using ByteArrayList = List<ByteArray>;

} // namespace

#include <cflib/base/impl/bytearrayimpl.h>

namespace std {
template<> struct hash<cflib::base::ByteArray> {
    size_t operator()(const cflib::base::ByteArray & ba) const {
        return hash<string_view>()(ba.sv());
    }
};
}

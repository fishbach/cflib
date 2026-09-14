/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base/container.h>

#include <string>
#include <string_view>

namespace cflib::base {

class ByteArray;
using ByteArrayList = List<ByteArray>;

// A ByteArray is a copy-on-write sequence of raw bytes.
// There is no encoding semantics. For UTF-8 text use String.
//
// Capacity is a power of two.
// Shrinking never reduces capacity.
//
// The buffer has always a '\0' after the accessable data,
// to make access to const char * cheap.
class ByteArray
{
public:
    ByteArray();
    ByteArray(const char  * data);
    ByteArray(const char  * data, size_t len);
    ByteArray(const uint8 * data, size_t len);
    explicit ByteArray(size_t n, char c = '\0');
    ~ByteArray();

    // implicit sharing
    ByteArray(const ByteArray & other);
    ByteArray(ByteArray && other);
    ByteArray & operator=(const ByteArray & other);
    ByteArray & operator=(ByteArray && other);
    void detach(size_t hint = 0);

    static ByteArray fromRawData(const void * data, size_t len);

    // null and size
    bool   isNull()   const;
    bool   isEmpty()  const;
    size_t size()     const;
    size_t length()   const;
    size_t capacity() const;

    // resize
    void resize(size_t n);
    void resize(size_t n, char c);
    void reserve(size_t n);
    void clear();

    // returns nullptr, if isNull()
    // '\0'-terminated C-string otherwise
    const char *  constCharPtr() const;
    char *        charPtr();
    const uint8 * constData() const;
    uint8 *       data();

    // std::string, std::string_view
    explicit ByteArray(std::string      s );
    explicit ByteArray(std::string_view sv);
    std::string      toStdString    () const;
    std::string_view toStdStringView() const;

    // char access
    char   at(size_t i) const;
    char   operator[](size_t i) const;
    char & operator[](size_t i);

    // search
    bool startsWith(const ByteArray & other) const;
    bool startsWith(const char * s) const;
    bool endsWith  (const ByteArray & other) const;
    bool endsWith  (const char * s) const;

    bool contains(const ByteArray & other) const;
    bool contains(char c) const;
    bool contains(const char * s) const;

    ssize_t indexOf(char c, size_t from = 0) const;
    ssize_t indexOf(const char * s, size_t from = 0) const;
    ssize_t indexOf(const ByteArray & other, size_t from = 0) const;

    // substring
    ByteArray left (size_t n) const;
    ByteArray right(size_t n) const;
    ByteArray mid  (size_t pos, ssize_t len = -1) const;

    // add, remove
    ByteArray & prepend(const char * s, size_t len);
    ByteArray & prepend(const char * s);

    ByteArray & insert(size_t pos, const ByteArray & ba);
    ByteArray & insert(size_t pos, const char * s, size_t len);

    ByteArray & append(char c);
    ByteArray & append(const char * s);
    ByteArray & append(const char * s, size_t len);
    ByteArray & append(const ByteArray & other);

    ByteArray & remove(size_t pos, size_t len);

    // change
    ByteArray & replace(char before, const char * after);
    ByteArray & replace(const char * before, const char * after);
    ByteArray & replace(size_t pos, size_t len, const char * newData);
    ByteArray & replace(size_t pos, size_t len, const char * newData, size_t newLen);

    ByteArray trimmed()    const;
    ByteArray simplified() const;
    ByteArray toLower()    const;

    // Numbers
    uint32 toUInt (bool * ok = nullptr) const;
    int32  toInt  (bool * ok = nullptr) const;
    uint64 toULong(bool * ok = nullptr) const;
    int64  toLong (bool * ok = nullptr) const;
    template <typename T>
    static ByteArray fromInt(T v);
    template <typename T>
    static ByteArray fromFloat(T v);

    // Hex
    ByteArray toHex() const;
    static ByteArray fromHex(const ByteArray & hex);
    static ByteArray fromHex(const char * hex);

    // Base64
    ByteArray toBase64() const;
    static ByteArray fromBase64(const ByteArray & base64);

    // split
    ByteArrayList split(char delim) const;

    ByteArray & operator+=(char c);
    ByteArray & operator+=(const char * s);
    ByteArray & operator+=(const ByteArray & o);

    ByteArray operator+(char c)              const;
    ByteArray operator+(const char * s)      const;
    ByteArray operator+(const ByteArray & o) const;

    // compare
    bool operator==(const ByteArray & o) const;
    bool operator!=(const ByteArray & o) const;
    bool operator< (const ByteArray & o) const;
    bool operator==(const char * o) const;
    bool operator!=(const char * o) const;

protected:
    struct Shared;
    Shared * d;
    void appendBytes(const char * s, size_t len);

private:
    inline static Shared * sharedNull();
    static void deleteShared(const Shared * s);
    void release();
};

} // namespace

// std::hash for ByteArray
namespace std {
template<> struct hash<cflib::base::ByteArray> {
    size_t operator()(const cflib::base::ByteArray & ba) const {
        return hash<string_view>()(ba.toStdStringView());
    }
};
}

#include <cflib/base/impl/bytearrayimpl.h>

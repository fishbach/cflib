/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base/bytearray.h>
#include <cflib/base/container.h>

#include <cstring>
#include <string>
#include <string_view>

namespace cflib::base {

class String;
using StringList = List<String>;

// A String is UTF-8 encoded text. Since UTF-8 text is a byte sequence,
// String is a ByteArray with text-specific operations layered on top.
// Everything that operates on bytes (append, indexOf, replace, Base64, Hex,
// ...) is inherited from ByteArray; only methods with text semantics or a
// String-typed result are redeclared here.
//
// Note: size() / length() count bytes, not Unicode characters. Use
// charCount() for the number of codepoints.

class String : public ByteArray
{
public:
    String() : ByteArray() {}
    String(const char * utf8) : ByteArray(utf8) {}
    String(const char * utf8, size_t len) : ByteArray(utf8, len) {}
    String(std::string s) : ByteArray(std::move(s)) {}
    String(size_t n, char c) : ByteArray(n, c) {}
    String(std::string_view sv) : ByteArray(sv) {}
    String(const ByteArray & ba) : ByteArray(ba) {}

    // A null string stays null when resized to zero, otherwise behave like
    // ByteArray::resize (which detaches and clears the null flag).
    void resize(size_t n) { if (isNull() && n == 0) return; ByteArray::resize(n); }
    using ByteArray::resize;

    // O(1) non-owning view over the UTF-8 bytes; copy via toStdString().
    // The buffer is not NUL-terminated, so there is no c_str(); pass
    // toStdString().c_str() where a C string is required.
    std::string_view str() const { return sv(); }
    size_t           byteSize() const { return size(); }

    // O(1): the UTF-8 byte sequence, shared (copy-on-write)
    ByteArray toUtf8()   const { return *this; }
    ByteArray toLatin1() const { return toUtf8(); }

    // Number of Unicode codepoints (UTF-8 scan, O(n))
    size_t charCount() const;

    ssize_t lastIndexOf(const char * s) const;

    size_t count(const String & subStr) const;

    String mid(size_t bytePos, size_t len = npos) const;
    String left(size_t n) const  { return mid(0, n); }
    String right(size_t n) const {
        if (n >= size()) return *this;
        return mid(size() - n);
    }

    String trimmed() const;
    String toLower() const;
    String toUpper() const;

    StringList split(char delim) const;
    StringList split(const char * delim) const;

    String join(const StringList & list) const;

    static String fromUtf8(const char * s, size_t len = npos);
    static String fromUtf8(const ByteArray & ba) { return String(ba); }
    static String fromLatin1(const char * s) { return s ? String(s) : String(); }

    bool operator<= (const String & o) const { return d->sv() <= o.d->sv(); }
    bool operator>  (const String & o) const { return d->sv() >  o.d->sv(); }
    bool operator>= (const String & o) const { return d->sv() >= o.d->sv(); }

    // libstdc++ 13 provides no operator+ for string_view (neither view+view
    // nor string+view), so the combined buffer is built via std::string
    String  operator+ (const String & o) const { return String(std::string(d->sv()) + std::string(o.d->sv())); }
    String  operator+ (const char * s)   const { return String(std::string(d->sv()) + s); }
    String  operator+ (char c)           const { String r(*this); r += c; return r; }
    String & operator+=(const String & o) {
        detach();
        d = d->append(o.constData(), o.size());
        d->setNull(false);
        return *this;
    }
    String & operator+=(const char * s)   {
        detach();
        d = d->append(s, std::strlen(s));
        d->setNull(false);
        return *this;
    }
    String & operator+=(char c)           {
        detach();
        d = d->append(&c, 1);
        d->setNull(false);
        return *this;
    }

    String & operator<<(const String & o)    { detach(); return *this += o; }
    String & operator<<(const ByteArray & o) { appendBytes(o.constData(), o.size()); return *this; }
    String & operator<<(const char * s)      { detach(); return *this += s; }
    String & operator<<(char c)              { detach(); return *this += c; }
};

} // namespace

namespace std {
template<> struct hash<cflib::base::String> {
    size_t operator()(const cflib::base::String & s) const {
        return hash<string_view>()(s.str());
    }
};
}

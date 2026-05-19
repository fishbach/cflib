/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/base/bytearray.h>
#include <cflib/util/test.h>

#include <cstring>
#include <format>
#include <string>
#include <vector>

using namespace cflib::base;

class ByteArray_test : public cflib::util::TestBase
{
public:
    std::vector<cflib::util::TestMethod> testMethods() const override {
        auto self = const_cast<ByteArray_test *>(this);
        return {
            {"test_constructors",                [self]() { self->test_constructors(); }},
            {"test_null_and_empty",              [self]() { self->test_null_and_empty(); }},
            {"test_accessors",                   [self]() { self->test_accessors(); }},
            {"test_resize_reserve_clear",        [self]() { self->test_resize_reserve_clear(); }},
            {"test_append_prepend",              [self]() { self->test_append_prepend(); }},
            {"test_insert",                      [self]() { self->test_insert(); }},
            {"test_mid_left_right",              [self]() { self->test_mid_left_right(); }},
            {"test_startsWith_endsWith",         [self]() { self->test_startsWith_endsWith(); }},
            {"test_contains",                    [self]() { self->test_contains(); }},
            {"test_indexOf",                     [self]() { self->test_indexOf(); }},
            {"test_replace",                     [self]() { self->test_replace(); }},
            {"test_trimmed",                     [self]() { self->test_trimmed(); }},
            {"test_toBase64_fromBase64",         [self]() { self->test_toBase64_fromBase64(); }},
            {"test_toHex_fromHex",               [self]() { self->test_toHex_fromHex(); }},
            {"test_numeric_conversions",         [self]() { self->test_numeric_conversions(); }},
            {"test_split",                       [self]() { self->test_split(); }},
            {"test_concatenation",               [self]() { self->test_concatenation(); }},
            {"test_implicit_sharing",            [self]() { self->test_implicit_sharing(); }},
        };
    }

private:
    // Construction tests
    void test_constructors()
    {
        // Default constructor
        ByteArray ba1;
        TVERIFY(ba1.isNull());
        TVERIFY(ba1.isEmpty());

        // Empty string (not null)
        ByteArray ba2("");
        TVERIFY(!ba2.isNull());
        TVERIFY(ba2.isEmpty());

        // From const char*
        ByteArray ba3("hello");
        TVERIFY(!ba3.isNull());
        TVERIFY(!ba3.isEmpty());
        TCOMPARE(ba3, ByteArray("hello"));

        // From const char* with length
        ByteArray ba4("hello world", 5);
        TCOMPARE(ba4, ByteArray("hello"));

        // From uint8*
        uint8 data[] = {1, 2, 3, 4};
        ByteArray ba5(data, 4);
        TCOMPARE(ba5.size(), (size_t)4);

        // From std::string
        std::string stdstr = "world";
        ByteArray ba6(stdstr);
        TCOMPARE(ba6, ByteArray("world"));

        // From std::string_view
        std::string_view sv = "view";
        ByteArray ba7(sv);
        TCOMPARE(ba7, ByteArray("view"));

        // From size_t n, char c
        ByteArray ba8(5, 'x');
        TCOMPARE(ba8, ByteArray("xxxxx"));

        // fromRawData
        ByteArray ba9 = ByteArray::fromRawData("raw", 3);
        TCOMPARE(ba9, ByteArray("raw"));
    }

    // Null and empty tests
    void test_null_and_empty()
    {
        ByteArray nullBa;
        ByteArray emptyBa("");

        TVERIFY(nullBa.isNull());
        TVERIFY(nullBa.isEmpty());

        TVERIFY(!emptyBa.isNull());
        TVERIFY(emptyBa.isEmpty());

        ByteArray fromData("");
        TVERIFY(!fromData.isNull());
        TVERIFY(fromData.isEmpty());

        ByteArray fromNull((const char*)nullptr);
        TVERIFY(fromNull.isNull());
        TVERIFY(fromNull.isEmpty());
    }

    // Accessor tests
    void test_accessors()
    {
        ByteArray ba("hello");

        TCOMPARE(ba.size(), (size_t)5);
        TCOMPARE(ba.length(), (size_t)5);
        TCOMPARE(strcmp(ba.constData(), "hello"), 0);
        TVERIFY(!ba.isEmpty());
        TVERIFY(!ba.isNull());

        // operator[]
        TCOMPARE(ba[0], 'h');
        TCOMPARE(ba[4], 'o');

        // Modification via operator[]
        ByteArray modifiable("abc");
        modifiable[0] = 'x';
        TCOMPARE(modifiable, ByteArray("xbc"));

        // at()
        TCOMPARE(ba.at(0), 'h');
        TCOMPARE(ba.at(4), 'o');
    }

    // Resize/reserve/clear tests
    void test_resize_reserve_clear()
    {
        ByteArray ba;

        // reserve
        ba.reserve(997);
        TVERIFY(ba.capacity() >= (size_t)997);

        // resize
        ba.resize(123);
        TCOMPARE(ba.size(), (size_t)123);
        TVERIFY(ba.capacity() >= (size_t)997);

        // resize with char
        ba.resize(50, 'y');
        TVERIFY(ba.size() >= (size_t)50);

        // clear
        ba.clear();
        TVERIFY(ba.isNull());
        TVERIFY(ba.isEmpty());

        // Empty string is not null
        TVERIFY(!ByteArray("").isNull());
    }

    // Append/Prepend tests
    void test_append_prepend()
    {
        ByteArray ba("hello");

        // append char
        ba.append(' ');
        TCOMPARE(ba, ByteArray("hello "));

        // append const char*
        ba.append("world");
        TCOMPARE(ba, ByteArray("hello world"));

        // append const char* with length
        ba.append("!!!", 2);
        TCOMPARE(ba, ByteArray("hello world!!"));

        // append ByteArray
        ba.append(ByteArray("?!"));
        TCOMPARE(ba, ByteArray("hello world!!?!"));

        // prepend const char* with length
        ByteArray ba2("world");
        ba2.prepend("hello ", 6);
        TCOMPARE(ba2, ByteArray("hello world"));

        // prepend const char*
        ByteArray ba3("test");
        ba3.prepend("pre");
        TCOMPARE(ba3, ByteArray("pretest"));

        // operator+=
        ba += '!';
        TVERIFY(ba.endsWith("!"));

        ba += "suffix";
        TVERIFY(ba.endsWith("suffix"));

        // operator<<
        ByteArray ba4;
        ba4 << "a" << "b" << "c";
        TCOMPARE(ba4, ByteArray("abc"));
    }

    // Insert tests
    void test_insert()
    {
        ByteArray ba("abcdef");

        // insert ByteArray
        ba.insert(2, ByteArray("xyz"));
        TCOMPARE(ba, ByteArray("abxyzcdef"));

        // insert const char* with length
        ByteArray ba2("hello");
        ba2.insert(0, "pre", 3);
        TCOMPARE(ba2, ByteArray("prehello"));

        // insert const char*
        ByteArray ba3("world");
        ba3.insert(0, "hello ");
        TCOMPARE(ba3, ByteArray("hello world"));
    }

    // Mid/Left/Right tests
    void test_mid_left_right()
    {
        ByteArray ba("hello world");

        // mid
        TCOMPARE(ba.mid(0), ByteArray("hello world"));
        TCOMPARE(ba.mid(6), ByteArray("world"));
        TCOMPARE(ba.mid(0, 5), ByteArray("hello"));
        TCOMPARE(ba.mid(6, 5), ByteArray("world"));
        TCOMPARE(ba.mid(11), ByteArray(""));
        TCOMPARE(ba.mid(20), ByteArray(""));

        // left
        TCOMPARE(ba.left(5), ByteArray("hello"));
        TCOMPARE(ba.left(11), ByteArray("hello world"));
        TCOMPARE(ba.left(20), ByteArray("hello world"));

        // right
        TCOMPARE(ba.right(5), ByteArray("world"));
        TCOMPARE(ba.right(11), ByteArray("hello world"));
        TCOMPARE(ba.right(20), ByteArray("hello world"));
    }

    // StartsWith/EndsWith tests
    void test_startsWith_endsWith()
    {
        ByteArray ba("hello world");

        TVERIFY(ba.startsWith("hello"));
        TVERIFY(ba.startsWith("hello world"));
        TVERIFY(!ba.startsWith("world"));
        TVERIFY(!ba.startsWith("HELLO"));

        // From ByteArray
        TVERIFY(ba.startsWith(ByteArray("hello")));
        TVERIFY(ba.endsWith(ByteArray("world")));

        TVERIFY(ba.endsWith("world"));
        TVERIFY(ba.endsWith("hello world"));
        TVERIFY(!ba.endsWith("hello"));
        TVERIFY(!ba.endsWith("WORLD"));
    }

    // Contains tests
    void test_contains()
    {
        ByteArray ba("hello world");

        TVERIFY(ba.contains("hello"));
        TVERIFY(ba.contains("world"));
        TVERIFY(ba.contains("lo wo"));
        TVERIFY(!ba.contains("xyz"));
        TVERIFY(!ba.contains("HELLO"));

        TVERIFY(ba.contains('h'));
        TVERIFY(ba.contains('w'));
        TVERIFY(!ba.contains('z'));
        TVERIFY(!ba.contains('H'));  // case sensitive
    }

    // IndexOf tests
    void test_indexOf()
    {
        ByteArray ba("hello world hello");

        // From char
        TCOMPARE(ba.indexOf('h'), (ssize_t)0);
        TCOMPARE(ba.indexOf('w'), (ssize_t)6);
        TCOMPARE(ba.indexOf('z'), (ssize_t)-1);

        // From const char*
        TCOMPARE(ba.indexOf("hello"), (ssize_t)0);
        TCOMPARE(ba.indexOf("world"), (ssize_t)6);
        TCOMPARE(ba.indexOf("xyz"), (ssize_t)-1);

        // From ByteArray
        TCOMPARE(ba.indexOf(ByteArray("hello")), (ssize_t)0);
        TCOMPARE(ba.indexOf(ByteArray("world")), (ssize_t)6);

        // With from offset
        TCOMPARE(ba.indexOf("hello", 1), (ssize_t)12);
        TVERIFY(ba.indexOf('l', 3) == 3);
        TVERIFY(ba.indexOf('l', 4) == 9);
    }

    // Replace tests
    void test_replace()
    {
        ByteArray ba("hello world hello");

        // replace(const char*, const char*)
        ba.replace("hello", "hi");
        TCOMPARE(ba, ByteArray("hi world hi"));

        // replace(char, const char*)
        ByteArray ba2("test");
        ba2.replace('t', "x");
        TCOMPARE(ba2, ByteArray("xesx"));

        // replace(size_t, size_t, const char*, size_t)
        ByteArray ba3("abcdef");
        ba3.replace(1, 3, "xyz", 3);
        TCOMPARE(ba3, ByteArray("axyzef"));

        // replace(size_t, size_t, const char*)
        ByteArray ba4("abcdef");
        ba4.replace(1, 3, "zzz");
        TCOMPARE(ba4, ByteArray("azzzef"));
    }

    // Trimmed test
    void test_trimmed()
    {
        TCOMPARE(ByteArray("  hello  ").trimmed(), ByteArray("hello"));
        TCOMPARE(ByteArray("\t\nhello\r\n  ").trimmed(), ByteArray("hello"));
        TCOMPARE(ByteArray("  \t  ").trimmed(), ByteArray(""));
        TCOMPARE(ByteArray(" hello world ").trimmed(), ByteArray("hello world"));
    }

    // Base64 tests
    void test_toBase64_fromBase64()
    {
        // Empty
        TCOMPARE(ByteArray().toBase64(), ByteArray(""));

        // Basic
        TCOMPARE(ByteArray("A").toBase64(), ByteArray("QQ=="));
        TCOMPARE(ByteArray("AB").toBase64(), ByteArray("QUI="));
        TCOMPARE(ByteArray("ABC").toBase64(), ByteArray("QUJD"));

        // Roundtrip
        ByteArray original("Hello World!");
        ByteArray encoded = original.toBase64();
        ByteArray decoded = ByteArray::fromBase64(encoded);
        TCOMPARE(decoded, original);
    }

    // Hex tests
    void test_toHex_fromHex()
    {
        // Empty
        TCOMPARE(ByteArray().toHex(), ByteArray(""));

        // Basic
        TVERIFY(ByteArray("A").toHex() == ByteArray("41"));
        TVERIFY(ByteArray("\xFF").toHex() == ByteArray("ff"));
        // Use ByteArray constructor with explicit length for binary data
        TVERIFY(ByteArray("\x00\xFF\x01", 3).toHex() == ByteArray("00ff01"));

        // fromHex
        TVERIFY(ByteArray::fromHex("41") == ByteArray("A"));
        TVERIFY(ByteArray::fromHex("4142") == ByteArray("AB"));
        TVERIFY(ByteArray::fromHex("00ff01") == ByteArray("\x00\xFF\x01", 3));

        // fromHex ignores whitespace
        TVERIFY(ByteArray::fromHex("41 42") == ByteArray("AB"));
        TVERIFY(ByteArray::fromHex("41\n42") == ByteArray("AB"));
    }

    // Numeric conversions tests
    void test_numeric_conversions()
    {
        bool ok;

        // toUInt
        TCOMPARE(ByteArray("42").toUInt(&ok), (uint32)42);
        TVERIFY(ok);
        TCOMPARE(ByteArray("0").toUInt(&ok), (uint32)0);
        TVERIFY(ok);
        TCOMPARE(ByteArray("abc").toUInt(&ok), (uint32)0);
        TVERIFY(!ok);

        // toInt
        TCOMPARE(ByteArray("-42").toInt(&ok), (int32)-42);
        TVERIFY(ok);
        TCOMPARE(ByteArray("abc").toInt(&ok), (int32)0);
        TVERIFY(!ok);

        // toULongLong
        TCOMPARE(ByteArray("123456789012345").toULongLong(&ok), (uint64)123456789012345LL);
        TVERIFY(ok);
        TCOMPARE(ByteArray("abc").toULongLong(&ok), (uint64)0);
        TVERIFY(!ok);
    }

    // Split test
    void test_split()
    {
        ByteArray ba("a,b,c,d");

        // Split by char
        auto parts1 = ba.split(',');
        TCOMPARE(parts1.size(), (size_t)4);
        TCOMPARE(parts1[0], ByteArray("a"));
        TCOMPARE(parts1[1], ByteArray("b"));
        TCOMPARE(parts1[2], ByteArray("c"));
        TCOMPARE(parts1[3], ByteArray("d"));

        // Empty string split
        auto parts2 = ByteArray("").split(',');
        TCOMPARE(parts2.size(), (size_t)1);
        TCOMPARE(parts2[0], ByteArray(""));

        // Single element without delimiter
        auto parts3 = ByteArray("abc").split(',');
        TCOMPARE(parts3.size(), (size_t)1);
        TCOMPARE(parts3[0], ByteArray("abc"));

        // Consecutive delimiters
        auto parts4 = ByteArray("a,,b").split(',');
        TCOMPARE(parts4.size(), (size_t)3);
        TCOMPARE(parts4[0], ByteArray("a"));
        TCOMPARE(parts4[1], ByteArray(""));
        TCOMPARE(parts4[2], ByteArray("b"));
    }

    // Concatenation tests
    void test_concatenation()
    {
        ByteArray ba1("hello");
        ByteArray ba2(" world");

        // operator+
        TCOMPARE(ba1 + ba2, ByteArray("hello world"));
        TCOMPARE(ba1 + " world", ByteArray("hello world"));
        TCOMPARE("hello" + ba2, ByteArray("hello world"));

        // operator+=
        ByteArray ba3("hello");
        ba3 += " world";
        TCOMPARE(ba3, ByteArray("hello world"));
    }

    // Implicit sharing tests
    void test_implicit_sharing()
    {
        ByteArray ba1("hello");
        ByteArray ba2 = ba1;

        // Initially share (same data)
        TCOMPARE(ba1, ba2);

        // Modify ba2 - should detach
        ba2[0] = 'x';
        TCOMPARE(ba1, ByteArray("hello"));
        TCOMPARE(ba2, ByteArray("xello"));

        // ba1 should not be affected by ba2's modification
        TVERIFY(ba1 != ba2);

        // Test detach on shared data
        ByteArray ba3("test");
        ByteArray ba4 = ba3;
        ba3.detach();
        ba3[0] = 'x';
        TCOMPARE(ba3, ByteArray("xest"));
        TCOMPARE(ba4, ByteArray("test"));
    }
};

ADD_TEST(ByteArray_test)

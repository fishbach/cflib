/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/base/string.h>
#include <cflib/util/test.h>

#include <cstring>
#include <memory>
#include <string>
#include <vector>

using namespace cflib::base;

class String_test : public cflib::util::TestBase
{
public:
    std::vector<cflib::util::TestMethod> testMethods() const override {
        auto self = const_cast<String_test *>(this);
        return {
            {"test_constructors",                  [self]() { self->test_constructors(); }},
            {"test_null_and_empty",                [self]() { self->test_null_and_empty(); }},
            {"test_accessors",                     [self]() { self->test_accessors(); }},
            {"test_indexOf",                       [self]() { self->test_indexOf(); }},
            {"test_contains",                      [self]() { self->test_contains(); }},
            {"test_startsWith_endsWith",           [self]() { self->test_startsWith_endsWith(); }},
            {"test_count",                         [self]() { self->test_count(); }},
            {"test_mid_left_right",                [self]() { self->test_mid_left_right(); }},
            {"test_trimmed_simplified",            [self]() { self->test_trimmed_simplified(); }},
            {"test_toLower_toUpper",               [self]() { self->test_toLower_toUpper(); }},
            {"test_split",                         [self]() { self->test_split(); }},
            {"test_replace",                       [self]() { self->test_replace(); }},
            {"test_toLong_toULong",                [self]() { self->test_toLong_toULong(); }},
            {"test_concatenation",                 [self]() { self->test_concatenation(); }},
            {"test_implicit_sharing",              [self]() { self->test_implicit_sharing(); }},
            {"test_utf8_charCount",                [self]() { self->test_utf8_charCount(); }},
            {"test_unicode",                       [self]() { self->test_unicode(); }},
        };
    }

private:
    // Construction tests
    void test_constructors()
    {
        // Default constructor
        String s1;
        TVERIFY(s1.isNull());
        TVERIFY(s1.isEmpty());

        // Empty string (not null)
        String s2("");
        TVERIFY(!s2.isNull());
        TVERIFY(s2.isEmpty());

        // From const char*
        String s3("hello");
        TVERIFY(!s3.isNull());
        TVERIFY(!s3.isEmpty());
        TCOMPARE(s3, String("hello"));

        // From const char* (nullptr)
        String s4((const char*)nullptr);
        TVERIFY(s4.isNull());

        // From std::string
        std::string stdstr = "world";
        String s5(stdstr);
        TCOMPARE(s5, String("world"));

        // From const char* with length
        String s6("hello world", 5);
        TCOMPARE(s6, String("hello"));

        // From std::string_view
        std::string_view sv = "view";
        String s7(sv);
        TCOMPARE(s7, String("view"));

        // From size_t n, char c
        String s8(5, 'x');
        TCOMPARE(s8, String("xxxxx"));

        // Number conversions
        TCOMPARE(String::number(42), String("42"));
        TCOMPARE(String::number(-42), String("-42"));
        TCOMPARE(String::number(9223372036854775807LL), String("9223372036854775807"));
        TCOMPARE(String::number(-9223372036854775807LL), String("-9223372036854775807"));
        TVERIFY(String::number(3.14159).indexOf("3.14") == 0);
        TVERIFY(String::number(2.71828).indexOf("2.71") == 0);
    }

    // Null and empty tests
    void test_null_and_empty()
    {
        String nullStr;
        String emptyStr("");

        TVERIFY(nullStr.isNull());
        TVERIFY(nullStr.isEmpty());

        TVERIFY(!emptyStr.isNull());
        TVERIFY(emptyStr.isEmpty());

        String fromData("");
        TVERIFY(!fromData.isNull());
        TVERIFY(fromData.isEmpty());

        String fromNull((const char*)nullptr);
        TVERIFY(fromNull.isNull());
        TVERIFY(fromNull.isEmpty());
    }

    // Accessor tests
    void test_accessors()
    {
        String s("hello");

        TCOMPARE(s.str(), std::string("hello"));
        TCOMPARE(strcmp(s.c_str(), "hello"), 0);
        TCOMPARE(s.byteSize(), (size_t)5);
        TCOMPARE(s.size(), (size_t)5);
        TCOMPARE(s.length(), (size_t)5);
        TVERIFY(!s.isEmpty());
        TVERIFY(!s.isNull());

        // operator[]
        TCOMPARE(s[0], 'h');
        TCOMPARE(s[4], 'o');

        // Modification via operator[]
        String modifiable("abc");
        modifiable[0] = 'x';
        TCOMPARE(modifiable, String("xbc"));

        // toUtf8 and toLatin1
        String hello("hello");
        ByteArray ba = hello.toUtf8();
        TCOMPARE(ba, ByteArray("hello"));
        TVERIFY(hello.toLatin1() == hello.toUtf8());
    }

    // indexOf tests
    void test_indexOf()
    {
        String s("hello world hello");

        // From const char*
        TCOMPARE(s.indexOf("hello"), (ssize_t)0);
        TCOMPARE(s.indexOf("world"), (ssize_t)6);
        TCOMPARE(s.indexOf("xyz"), (ssize_t)-1);

        // From String
        TCOMPARE(s.indexOf(String("hello")), (ssize_t)0);
        TCOMPARE(s.indexOf(String("world")), (ssize_t)6);

        // From char
        TCOMPARE(s.indexOf('h'), (ssize_t)0);
        TCOMPARE(s.indexOf('w'), (ssize_t)6);
        TCOMPARE(s.indexOf('z'), (ssize_t)-1);

        // With from offset
        TCOMPARE(s.indexOf("hello", 1), (ssize_t)12);
        TCOMPARE(s.indexOf("hello", 13), (ssize_t)-1);
        TCOMPARE(s.indexOf('l', 3), (ssize_t)3);
        TCOMPARE(s.indexOf('l', 4), (ssize_t)9);

        // lastIndexOf (const char* only, no lastIndexOf(char))
        TCOMPARE(s.lastIndexOf("hello"), (ssize_t)12);
        TCOMPARE(s.lastIndexOf("world"), (ssize_t)6);
    }

    // contains tests
    void test_contains()
    {
        String s("hello world");

        TVERIFY(s.contains("hello"));
        TVERIFY(s.contains("world"));
        TVERIFY(s.contains("lo wo"));
        TVERIFY(!s.contains("xyz"));
        TVERIFY(!s.contains("HELLO"));

        TVERIFY(s.contains('h'));
        TVERIFY(s.contains('w'));
        TVERIFY(!s.contains('z'));
        TVERIFY(!s.contains('H'));  // case sensitive
    }

    // startsWith and endsWith tests
    void test_startsWith_endsWith()
    {
        String s("hello world");

        TVERIFY(s.startsWith("hello"));
        TVERIFY(s.startsWith("hello world"));
        TVERIFY(!s.startsWith("world"));
        TVERIFY(!s.startsWith("HELLO"));

        TVERIFY(s.endsWith("world"));
        TVERIFY(s.endsWith("hello world"));
        TVERIFY(!s.endsWith("hello"));
        TVERIFY(!s.endsWith("WORLD"));

        // From String
        TVERIFY(s.startsWith(String("hello")));
        TVERIFY(s.endsWith(String("world")));
    }

    // count tests
    void test_count()
    {
        String s("hello hello hello");

        TCOMPARE(s.count("hello"), (size_t)3);
        TCOMPARE(s.count("h"), (size_t)3);
        TCOMPARE(s.count(" "), (size_t)2);
        TCOMPARE(s.count("xyz"), (size_t)0);
        TCOMPARE(s.count("hello hello"), (size_t)1);
    }

    // mid, left, right tests
    void test_mid_left_right()
    {
        String s("hello world");

        // mid
        TCOMPARE(s.mid(0), String("hello world"));
        TCOMPARE(s.mid(6), String("world"));
        TCOMPARE(s.mid(0, 5), String("hello"));
        TCOMPARE(s.mid(6, 5), String("world"));
        TCOMPARE(s.mid(11), String(""));  // at end
        TCOMPARE(s.mid(20), String(""));  // beyond end

        // left
        TCOMPARE(s.left(5), String("hello"));
        TCOMPARE(s.left(11), String("hello world"));
        TCOMPARE(s.left(20), String("hello world"));

        // right
        TCOMPARE(s.right(5), String("world"));
        TCOMPARE(s.right(11), String("hello world"));
        TCOMPARE(s.right(20), String("hello world"));
    }

    // trimmed and simplified tests
    void test_trimmed_simplified()
    {
        // trimmed
        TCOMPARE(String("  hello  ").trimmed(), String("hello"));
        TCOMPARE(String("\t\nhello\r\n  ").trimmed(), String("hello"));
        TCOMPARE(String("  \t  ").trimmed(), String(""));
        TCOMPARE(String(" hello world ").trimmed(), String("hello world"));

        // simplified
        TCOMPARE(String("  hello  ").simplified(), String("hello"));
        TCOMPARE(String("  hello   world  ").simplified(), String("hello world"));
        TCOMPARE(String("\t\nhello\r\n  world\t").simplified(), String("hello world"));
        TCOMPARE(String("  \t  ").simplified(), String(""));
        TCOMPARE(String("hello").simplified(), String("hello"));
    }

    // toLower and toUpper tests
    void test_toLower_toUpper()
    {
        String s("Hello World");

        TCOMPARE(s.toLower(), String("hello world"));
        TCOMPARE(s.toLower().toUpper(), String("HELLO WORLD"));

        // Already lowercase
        TCOMPARE(String("hello").toLower(), String("hello"));

        // Already uppercase
        TCOMPARE(String("HELLO").toUpper(), String("HELLO"));

        // Mixed case
        TCOMPARE(String("hElLo").toLower(), String("hello"));
        TCOMPARE(String("hElLo").toUpper(), String("HELLO"));
    }

    // split tests
    void test_split()
    {
        String s("a,b,c,d");

        // Split by char
        auto parts1 = s.split(',');
        TCOMPARE(parts1.size(), (size_t)4);
        TCOMPARE(parts1[0], String("a"));
        TCOMPARE(parts1[1], String("b"));
        TCOMPARE(parts1[2], String("c"));
        TCOMPARE(parts1[3], String("d"));

        // Split by const char*
        auto parts2 = s.split(",");
        TCOMPARE(parts2.size(), (size_t)4);

        // Empty string split
        auto parts3 = String("").split(',');
        TCOMPARE(parts3.size(), (size_t)1);
        TCOMPARE(parts3[0], String(""));

        // Single element without delimiter
        auto parts4 = String("abc").split(',');
        TCOMPARE(parts4.size(), (size_t)1);
        TCOMPARE(parts4[0], String("abc"));

        // Consecutive delimiters
        auto parts5 = String("a,,b").split(',');
        TCOMPARE(parts5.size(), (size_t)3);
        TCOMPARE(parts5[0], String("a"));
        TCOMPARE(parts5[1], String(""));
        TCOMPARE(parts5[2], String("b"));

        // Trailing delimiter
        auto parts6 = String("a,b,").split(',');
        TCOMPARE(parts6.size(), (size_t)3);
        TCOMPARE(parts6[0], String("a"));
        TCOMPARE(parts6[1], String("b"));
        TCOMPARE(parts6[2], String(""));
    }

    // replace tests
    void test_replace()
    {
        String s("hello world hello");

        // replace(const char*, const char*)
        s.replace("hello", "hi");
        TCOMPARE(s, String("hi world hi"));

        // replace(size_t, size_t, const char*) - replaces len chars at pos
        String s2("abcdef");
        s2.replace(1, 3, "xyz");  // replaces positions 1,2,3 (bcd) with xyz -> axyzef
        TCOMPARE(s2, String("axyzef"));

        // Test that replace modifies in place
        String s3("test");
        s3.replace("t", "x");
        TCOMPARE(s3, String("xesx"));
    }

    // toLong and toULong tests
    void test_toLong_toULong()
    {
        bool ok;

        // Valid positive
        TCOMPARE(String("42").toLong(&ok), (int64)42);
        TVERIFY(ok);

        // Valid negative
        TCOMPARE(String("-42").toLong(&ok), (int64)-42);
        TVERIFY(ok);

        // Valid unsigned
        TCOMPARE(String("42").toULong(&ok), (uint64)42);
        TVERIFY(ok);

        // Invalid - empty string
        TCOMPARE(String("").toLong(&ok), (int64)0);
        TVERIFY(!ok);

        // Invalid - non-numeric
        TCOMPARE(String("abc").toLong(&ok), (int64)0);
        TVERIFY(!ok);

        // Invalid - mixed
        TCOMPARE(String("123abc").toLong(&ok), (int64)123);
        TVERIFY(!ok);  // should fail because of trailing characters

        // Valid with trailing space is ok (stops at space)
        TCOMPARE(String("42 ").toLong(&ok), (int64)42);
        TVERIFY(!ok);  // trailing space means not fully consumed

        // Large numbers
        TCOMPARE(String("9223372036854775807").toLong(&ok), (int64)9223372036854775807LL);
        TVERIFY(ok);
        TCOMPARE(String("-9223372036854775807").toLong(&ok), (int64)-9223372036854775807LL);
        TVERIFY(ok);
    }

    // Concatenation tests
    void test_concatenation()
    {
        String s1("hello");
        String s2(" world");

        // operator+
        TCOMPARE(s1 + s2, String("hello world"));
        TCOMPARE(s1 + " world", String("hello world"));
        TCOMPARE("hello" + s2, String("hello world"));

        // operator+=
        String s3("hello");
        s3 += " world";
        TCOMPARE(s3, String("hello world"));

        // operator<<
        String s4("hello");
        s4 << " world";
        TCOMPARE(s4, String("hello world"));

        // Chaining
        String s5;
        s5 << "a" << "b" << "c";
        TCOMPARE(s5, String("abc"));
    }

    // Implicit sharing tests
    void test_implicit_sharing()
    {
        String s1("hello");
        String s2 = s1;  // Copy, should share

        // Initially share (same data)
        TCOMPARE(s1, s2);

        // Modify s2 - should detach
        s2[0] = 'x';
        TCOMPARE(s1, String("hello"));
        TCOMPARE(s2, String("xello"));

        // s1 should not be affected by s2's modification
        TVERIFY(s1 != s2);

        // Test detach on shared data
        String s3("test");
        String s4 = s3;
        s3.detach();  // Explicit detach
        s3[0] = 'x';
        TCOMPARE(s3, String("xest"));
        TCOMPARE(s4, String("test"));
    }

    // UTF-8 charCount tests
    void test_utf8_charCount()
    {
        // ASCII: 1 byte per char
        TCOMPARE(String("hello").charCount(), (size_t)5);

        // UTF-8: 2 bytes per char (ö)
        TCOMPARE(String("hö").charCount(), (size_t)2);

        // UTF-8: 3 bytes per char (Chinese)
        // "中" is 0xE4 0xB8 0xAD
        TCOMPARE(String("中").charCount(), (size_t)1);

        // UTF-8: 4 bytes per char (emoji)
        // "😀" is 0xF0 0x9F 0x98 0x80
        TCOMPARE(String("😀").charCount(), (size_t)1);

        // Mixed
        // "hello世界😀" = 5 ASCII + 2 Chinese + 1 emoji = 8 chars
        String mixed("hello世界😀");
        TCOMPARE(mixed.charCount(), (size_t)8);
    }

    // Unicode tests
    void test_unicode()
    {
        // German umlauts
        String german("Größe");
        TVERIFY(german.indexOf("größe") == -1);  // case sensitive
        TVERIFY(german.contains("ß"));
        TCOMPARE(german.toLower(), String("größe"));

        // Chinese characters
        String chinese("你好世界");
        TVERIFY(chinese.contains("你好"));
        TCOMPARE(chinese.charCount(), (size_t)4);

        // Emoji
        String emoji("Hello 😀 World 🎉");
        TVERIFY(emoji.contains("😀"));
        TVERIFY(emoji.contains("🎉"));

        // Build from UTF-8 bytes
        // ö = 0xC3 0xB6
        String fromUtf8 = String::fromUtf8("\xC3\xB6");
        TCOMPARE(fromUtf8, String("ö"));

        // Build from ByteArray
        ByteArray ba("\xC3\xB6", 2);
        String fromBa = String::fromUtf8(ba);
        TCOMPARE(fromBa, String("ö"));
    }
};

ADD_TEST(String_test)

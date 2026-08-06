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

TEST_SUITE("String") {

// Construction tests
TEST_CASE("String: constructors")
{
    // Default constructor
    String s1;
    REQUIRE(s1.isNull());
    REQUIRE(s1.isEmpty());

    // Empty string (not null)
    String s2("");
    REQUIRE(!s2.isNull());
    REQUIRE(s2.isEmpty());

    // From const char*
    String s3("hello");
    REQUIRE(!s3.isNull());
    REQUIRE(!s3.isEmpty());
    REQUIRE_EQ(s3, String("hello"));

    // From const char* (nullptr)
    String s4((const char*)nullptr);
    REQUIRE(s4.isNull());

    // From std::string
    std::string stdstr = "world";
    String s5(stdstr);
    REQUIRE_EQ(s5, String("world"));

    // From const char* with length
    String s6("hello world", 5);
    REQUIRE_EQ(s6, String("hello"));

    // From std::string_view
    std::string_view sv = "view";
    String s7(sv);
    REQUIRE_EQ(s7, String("view"));

    // From size_t n, char c
    String s8(5, 'x');
    REQUIRE_EQ(s8, String("xxxxx"));

    // Number conversions
    REQUIRE_EQ(String::number(42), String("42"));
    REQUIRE_EQ(String::number(-42), String("-42"));
    REQUIRE_EQ(String::number(9223372036854775807LL), String("9223372036854775807"));
    REQUIRE_EQ(String::number(-9223372036854775807LL), String("-9223372036854775807"));
    REQUIRE(String::number(3.14159).indexOf("3.14") == 0);
    REQUIRE(String::number(2.71828).indexOf("2.71") == 0);
}

// Null and empty tests
TEST_CASE("String: null_and_empty")
{
    String nullStr;
    String emptyStr("");

    REQUIRE(nullStr.isNull());
    REQUIRE(nullStr.isEmpty());

    REQUIRE(!emptyStr.isNull());
    REQUIRE(emptyStr.isEmpty());

    String fromData("");
    REQUIRE(!fromData.isNull());
    REQUIRE(fromData.isEmpty());

    String fromNull((const char*)nullptr);
    REQUIRE(fromNull.isNull());
    REQUIRE(fromNull.isEmpty());
}

// Accessor tests
TEST_CASE("String: accessors")
{
    String s("hello");

    REQUIRE_EQ(s.str(), std::string("hello"));
    REQUIRE_EQ(strcmp(s.c_str(), "hello"), 0);
    REQUIRE_EQ(s.byteSize(), (size_t)5);
    REQUIRE_EQ(s.size(), (size_t)5);
    REQUIRE_EQ(s.length(), (size_t)5);
    REQUIRE(!s.isEmpty());
    REQUIRE(!s.isNull());

    // operator[]
    REQUIRE_EQ(s[0], 'h');
    REQUIRE_EQ(s[4], 'o');

    // Modification via operator[]
    String modifiable("abc");
    modifiable[0] = 'x';
    REQUIRE_EQ(modifiable, String("xbc"));

    // toUtf8 and toLatin1
    String hello("hello");
    ByteArray ba = hello.toUtf8();
    REQUIRE_EQ(ba, ByteArray("hello"));
    REQUIRE(hello.toLatin1() == hello.toUtf8());
}

// indexOf tests
TEST_CASE("String: indexOf")
{
    String s("hello world hello");

    // From const char*
    REQUIRE_EQ(s.indexOf("hello"), (ssize_t)0);
    REQUIRE_EQ(s.indexOf("world"), (ssize_t)6);
    REQUIRE_EQ(s.indexOf("xyz"), (ssize_t)-1);

    // From String
    REQUIRE_EQ(s.indexOf(String("hello")), (ssize_t)0);
    REQUIRE_EQ(s.indexOf(String("world")), (ssize_t)6);

    // From char
    REQUIRE_EQ(s.indexOf('h'), (ssize_t)0);
    REQUIRE_EQ(s.indexOf('w'), (ssize_t)6);
    REQUIRE_EQ(s.indexOf('z'), (ssize_t)-1);

    // With from offset
    REQUIRE_EQ(s.indexOf("hello", 1), (ssize_t)12);
    REQUIRE_EQ(s.indexOf("hello", 13), (ssize_t)-1);
    REQUIRE_EQ(s.indexOf('l', 3), (ssize_t)3);
    REQUIRE_EQ(s.indexOf('l', 4), (ssize_t)9);

    // lastIndexOf (const char* only, no lastIndexOf(char))
    REQUIRE_EQ(s.lastIndexOf("hello"), (ssize_t)12);
    REQUIRE_EQ(s.lastIndexOf("world"), (ssize_t)6);
}

// contains tests
TEST_CASE("String: contains")
{
    String s("hello world");

    REQUIRE(s.contains("hello"));
    REQUIRE(s.contains("world"));
    REQUIRE(s.contains("lo wo"));
    REQUIRE(!s.contains("xyz"));
    REQUIRE(!s.contains("HELLO"));

    REQUIRE(s.contains('h'));
    REQUIRE(s.contains('w'));
    REQUIRE(!s.contains('z'));
    REQUIRE(!s.contains('H'));  // case sensitive
}

// startsWith and endsWith tests
TEST_CASE("String: startsWith_endsWith")
{
    String s("hello world");

    REQUIRE(s.startsWith("hello"));
    REQUIRE(s.startsWith("hello world"));
    REQUIRE(!s.startsWith("world"));
    REQUIRE(!s.startsWith("HELLO"));

    REQUIRE(s.endsWith("world"));
    REQUIRE(s.endsWith("hello world"));
    REQUIRE(!s.endsWith("hello"));
    REQUIRE(!s.endsWith("WORLD"));

    // From String
    REQUIRE(s.startsWith(String("hello")));
    REQUIRE(s.endsWith(String("world")));
}

// count tests
TEST_CASE("String: count")
{
    String s("hello hello hello");

    REQUIRE_EQ(s.count("hello"), (size_t)3);
    REQUIRE_EQ(s.count("h"), (size_t)3);
    REQUIRE_EQ(s.count(" "), (size_t)2);
    REQUIRE_EQ(s.count("xyz"), (size_t)0);
    REQUIRE_EQ(s.count("hello hello"), (size_t)1);
}

// mid, left, right tests
TEST_CASE("String: mid_left_right")
{
    String s("hello world");

    // mid
    REQUIRE_EQ(s.mid(0), String("hello world"));
    REQUIRE_EQ(s.mid(6), String("world"));
    REQUIRE_EQ(s.mid(0, 5), String("hello"));
    REQUIRE_EQ(s.mid(6, 5), String("world"));
    REQUIRE_EQ(s.mid(11), String(""));  // at end
    REQUIRE_EQ(s.mid(20), String(""));  // beyond end

    // left
    REQUIRE_EQ(s.left(5), String("hello"));
    REQUIRE_EQ(s.left(11), String("hello world"));
    REQUIRE_EQ(s.left(20), String("hello world"));

    // right
    REQUIRE_EQ(s.right(5), String("world"));
    REQUIRE_EQ(s.right(11), String("hello world"));
    REQUIRE_EQ(s.right(20), String("hello world"));
}

// trimmed and simplified tests
TEST_CASE("String: trimmed_simplified")
{
    // trimmed
    REQUIRE_EQ(String("  hello  ").trimmed(), String("hello"));
    REQUIRE_EQ(String("\t\nhello\r\n  ").trimmed(), String("hello"));
    REQUIRE_EQ(String("  \t  ").trimmed(), String(""));
    REQUIRE_EQ(String(" hello world ").trimmed(), String("hello world"));

    // simplified
    REQUIRE_EQ(String("  hello  ").simplified(), String("hello"));
    REQUIRE_EQ(String("  hello   world  ").simplified(), String("hello world"));
    REQUIRE_EQ(String("\t\nhello\r\n  world\t").simplified(), String("hello world"));
    REQUIRE_EQ(String("  \t  ").simplified(), String(""));
    REQUIRE_EQ(String("hello").simplified(), String("hello"));
}

// toLower and toUpper tests
TEST_CASE("String: toLower_toUpper")
{
    String s("Hello World");

    REQUIRE_EQ(s.toLower(), String("hello world"));
    REQUIRE_EQ(s.toLower().toUpper(), String("HELLO WORLD"));

    // Already lowercase
    REQUIRE_EQ(String("hello").toLower(), String("hello"));

    // Already uppercase
    REQUIRE_EQ(String("HELLO").toUpper(), String("HELLO"));

    // Mixed case
    REQUIRE_EQ(String("hElLo").toLower(), String("hello"));
    REQUIRE_EQ(String("hElLo").toUpper(), String("HELLO"));
}

// split tests
TEST_CASE("String: split")
{
    String s("a,b,c,d");

    // Split by char
    auto parts1 = s.split(',');
    REQUIRE_EQ(parts1.size(), (size_t)4);
    REQUIRE_EQ(parts1[0], String("a"));
    REQUIRE_EQ(parts1[1], String("b"));
    REQUIRE_EQ(parts1[2], String("c"));
    REQUIRE_EQ(parts1[3], String("d"));

    // Split by const char*
    auto parts2 = s.split(",");
    REQUIRE_EQ(parts2.size(), (size_t)4);

    // Empty string split
    auto parts3 = String("").split(',');
    REQUIRE_EQ(parts3.size(), (size_t)1);
    REQUIRE_EQ(parts3[0], String(""));

    // Single element without delimiter
    auto parts4 = String("abc").split(',');
    REQUIRE_EQ(parts4.size(), (size_t)1);
    REQUIRE_EQ(parts4[0], String("abc"));

    // Consecutive delimiters
    auto parts5 = String("a,,b").split(',');
    REQUIRE_EQ(parts5.size(), (size_t)3);
    REQUIRE_EQ(parts5[0], String("a"));
    REQUIRE_EQ(parts5[1], String(""));
    REQUIRE_EQ(parts5[2], String("b"));

    // Trailing delimiter
    auto parts6 = String("a,b,").split(',');
    REQUIRE_EQ(parts6.size(), (size_t)3);
    REQUIRE_EQ(parts6[0], String("a"));
    REQUIRE_EQ(parts6[1], String("b"));
    REQUIRE_EQ(parts6[2], String(""));
}

// replace tests
TEST_CASE("String: replace")
{
    String s("hello world hello");

    // replace(const char*, const char*)
    s.replace("hello", "hi");
    REQUIRE_EQ(s, String("hi world hi"));

    // replace(size_t, size_t, const char*) - replaces len chars at pos
    String s2("abcdef");
    s2.replace(1, 3, "xyz");  // replaces positions 1,2,3 (bcd) with xyz -> axyzef
    REQUIRE_EQ(s2, String("axyzef"));

    // Test that replace modifies in place
    String s3("test");
    s3.replace("t", "x");
    REQUIRE_EQ(s3, String("xesx"));
}

// toLong and toULong tests
TEST_CASE("String: toLong_toULong")
{
    bool ok;

    // Valid positive
    REQUIRE_EQ(String("42").toLong(&ok), (int64)42);
    REQUIRE(ok);

    // Valid negative
    REQUIRE_EQ(String("-42").toLong(&ok), (int64)-42);
    REQUIRE(ok);

    // Valid unsigned
    REQUIRE_EQ(String("42").toULong(&ok), (uint64)42);
    REQUIRE(ok);

    // Invalid - empty string
    REQUIRE_EQ(String("").toLong(&ok), (int64)0);
    REQUIRE(!ok);

    // Invalid - non-numeric
    REQUIRE_EQ(String("abc").toLong(&ok), (int64)0);
    REQUIRE(!ok);

    // Invalid - mixed
    REQUIRE_EQ(String("123abc").toLong(&ok), (int64)123);
    REQUIRE(!ok);  // should fail because of trailing characters

    // Valid with trailing space is ok (stops at space)
    REQUIRE_EQ(String("42 ").toLong(&ok), (int64)42);
    REQUIRE(!ok);  // trailing space means not fully consumed

    // Large numbers
    REQUIRE_EQ(String("9223372036854775807").toLong(&ok), (int64)9223372036854775807LL);
    REQUIRE(ok);
    REQUIRE_EQ(String("-9223372036854775807").toLong(&ok), (int64)-9223372036854775807LL);
    REQUIRE(ok);
}

// Concatenation tests
TEST_CASE("String: concatenation")
{
    String s1("hello");
    String s2(" world");

    // operator+
    REQUIRE_EQ(s1 + s2, String("hello world"));
    REQUIRE_EQ(s1 + " world", String("hello world"));
    REQUIRE_EQ("hello" + s2, String("hello world"));

    // operator+=
    String s3("hello");
    s3 += " world";
    REQUIRE_EQ(s3, String("hello world"));

    // operator<<
    String s4("hello");
    s4 << " world";
    REQUIRE_EQ(s4, String("hello world"));

    // Chaining
    String s5;
    s5 << "a" << "b" << "c";
    REQUIRE_EQ(s5, String("abc"));
}

// Implicit sharing tests
TEST_CASE("String: implicit_sharing")
{
    String s1("hello");
    String s2 = s1;  // Copy, should share

    // Initially share (same data)
    REQUIRE_EQ(s1, s2);

    // Verify const pointers point to same address while sharing
    const char * cstr1 = s1.c_str();
    const char * cstr2 = s2.c_str();
    REQUIRE(cstr1 == cstr2);

    // Modify s2 - should detach
    s2[0] = 'x';
    REQUIRE_EQ(s1, String("hello"));
    REQUIRE_EQ(s2, String("xello"));

    // s1 should not be affected by s2's modification
    REQUIRE(s1 != s2);

    // Verify const pointers now point to different addresses after detach
    REQUIRE(s1.c_str() != s2.c_str());

    // Test detach on shared data
    String s3("test");
    String s4 = s3;
    s3.detach();  // Explicit detach
    s3[0] = 'x';
    REQUIRE_EQ(s3, String("xest"));
    REQUIRE_EQ(s4, String("test"));

    // Verify const pointers are different after explicit detach
    REQUIRE(s3.c_str() != s4.c_str());
}

// UTF-8 charCount tests
TEST_CASE("String: utf8_charCount")
{
    // ASCII: 1 byte per char
    REQUIRE_EQ(String("hello").charCount(), (size_t)5);

    // UTF-8: 2 bytes per char (ö)
    REQUIRE_EQ(String("hö").charCount(), (size_t)2);

    // UTF-8: 3 bytes per char (Chinese)
    // "中" is 0xE4 0xB8 0xAD
    REQUIRE_EQ(String("中").charCount(), (size_t)1);

    // UTF-8: 4 bytes per char (emoji)
    // "😀" is 0xF0 0x9F 0x98 0x80
    REQUIRE_EQ(String("😀").charCount(), (size_t)1);

    // Mixed
    // "hello世界😀" = 5 ASCII + 2 Chinese + 1 emoji = 8 chars
    String mixed("hello世界😀");
    REQUIRE_EQ(mixed.charCount(), (size_t)8);
}

// Unicode tests
TEST_CASE("String: unicode")
{
    // German umlauts
    String german("Größe");
    REQUIRE(german.indexOf("größe") == -1);  // case sensitive
    REQUIRE(german.contains("ß"));
    REQUIRE_EQ(german.toLower(), String("größe"));

    // Chinese characters
    String chinese("你好世界");
    REQUIRE(chinese.contains("你好"));
    REQUIRE_EQ(chinese.charCount(), (size_t)4);

    // Emoji
    String emoji("Hello 😀 World 🎉");
    REQUIRE(emoji.contains("😀"));
    REQUIRE(emoji.contains("🎉"));

    // Build from UTF-8 bytes
    // ö = 0xC3 0xB6
    String fromUtf8 = String::fromUtf8("\xC3\xB6");
    REQUIRE_EQ(fromUtf8, String("ö"));

    // Build from ByteArray
    ByteArray ba("\xC3\xB6", 2);
    String fromBa = String::fromUtf8(ba);
    REQUIRE_EQ(fromBa, String("ö"));
}

}

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

TEST_SUITE("ByteArray") {

// Construction tests
TEST_CASE("ByteArray: constructors")
{
    // Default constructor
    ByteArray ba1;
    REQUIRE(ba1.isNull());
    REQUIRE(ba1.isEmpty());

    // Empty string (not null)
    ByteArray ba2("");
    REQUIRE(!ba2.isNull());
    REQUIRE(ba2.isEmpty());

    // From const char*
    ByteArray ba3("hello");
    REQUIRE(!ba3.isNull());
    REQUIRE(!ba3.isEmpty());
    REQUIRE_EQ(ba3, ByteArray("hello"));

    // From const char* with length
    ByteArray ba4("hello world", 5);
    REQUIRE_EQ(ba4, ByteArray("hello"));

    // From uint8*
    uint8 data[] = {1, 2, 3, 4};
    ByteArray ba5(data, 4);
    REQUIRE_EQ(ba5.size(), (size_t)4);

    // From std::string
    std::string stdstr = "world";
    ByteArray ba6(stdstr);
    REQUIRE_EQ(ba6, ByteArray("world"));

    // From std::string_view
    std::string_view sv = "view";
    ByteArray ba7(sv);
    REQUIRE_EQ(ba7, ByteArray("view"));

    // From size_t n, char c
    ByteArray ba8(5, 'x');
    REQUIRE_EQ(ba8, ByteArray("xxxxx"));

    // fromRawData
    ByteArray ba9 = ByteArray::fromRawData("raw", 3);
    REQUIRE_EQ(ba9, ByteArray("raw"));
}

// Null and empty tests
TEST_CASE("ByteArray: null_and_empty")
{
    ByteArray nullBa;
    ByteArray emptyBa("");

    REQUIRE(nullBa.isNull());
    REQUIRE(nullBa.isEmpty());

    REQUIRE(!emptyBa.isNull());
    REQUIRE(emptyBa.isEmpty());

    ByteArray fromData("");
    REQUIRE(!fromData.isNull());
    REQUIRE(fromData.isEmpty());

    ByteArray fromNull((const char*)nullptr);
    REQUIRE(fromNull.isNull());
    REQUIRE(fromNull.isEmpty());
}

// Accessor tests
TEST_CASE("ByteArray: accessors")
{
    ByteArray ba("hello");

    REQUIRE_EQ(ba.size(), (size_t)5);
    REQUIRE_EQ(ba.length(), (size_t)5);
    REQUIRE(ba == "hello");
    REQUIRE(!ba.isEmpty());
    REQUIRE(!ba.isNull());

    // operator[]
    REQUIRE_EQ(ba[0], 'h');
    REQUIRE_EQ(ba[4], 'o');

    // Modification via operator[]
    ByteArray modifiable("abc");
    modifiable[0] = 'x';
    REQUIRE_EQ(modifiable, ByteArray("xbc"));

    // at()
    REQUIRE_EQ(ba.at(0), 'h');
    REQUIRE_EQ(ba.at(4), 'o');
}

// Resize/reserve/clear tests
TEST_CASE("ByteArray: resize_reserve_clear")
{
    ByteArray ba;

    // reserve
    ba.reserve(997);
    REQUIRE(ba.capacity() >= (size_t)997);

    // resize
    ba.resize(123);
    REQUIRE_EQ(ba.size(), (size_t)123);
    REQUIRE(ba.capacity() >= (size_t)997);

    // resize with char
    ba.resize(50, 'y');
    REQUIRE(ba.size() >= (size_t)50);

    // clear
    ba.clear();
    REQUIRE(ba.isNull());
    REQUIRE(ba.isEmpty());

    // Empty string is not null
    REQUIRE(!ByteArray("").isNull());
}

// Append/Prepend tests
TEST_CASE("ByteArray: append_prepend")
{
    ByteArray ba("hello");

    // append char
    ba.append(' ');
    REQUIRE_EQ(ba, ByteArray("hello "));

    // append const char*
    ba.append("world");
    REQUIRE_EQ(ba, ByteArray("hello world"));

    // append const char* with length
    ba.append("!!!", 2);
    REQUIRE_EQ(ba, ByteArray("hello world!!"));

    // append ByteArray
    ba.append(ByteArray("?!"));
    REQUIRE_EQ(ba, ByteArray("hello world!!?!"));

    // prepend const char* with length
    ByteArray ba2("world");
    ba2.prepend("hello ", 6);
    REQUIRE_EQ(ba2, ByteArray("hello world"));

    // prepend const char*
    ByteArray ba3("test");
    ba3.prepend("pre");
    REQUIRE_EQ(ba3, ByteArray("pretest"));

    // operator+=
    ba += '!';
    REQUIRE(ba.endsWith("!"));

    ba += "suffix";
    REQUIRE(ba.endsWith("suffix"));

    // operator<<
    ByteArray ba4;
    ba4 << "a" << "b" << "c";
    REQUIRE_EQ(ba4, ByteArray("abc"));
}

// Insert tests
TEST_CASE("ByteArray: insert")
{
    ByteArray ba("abcdef");

    // insert ByteArray
    ba.insert(2, ByteArray("xyz"));
    REQUIRE_EQ(ba, ByteArray("abxyzcdef"));

    // insert const char* with length
    ByteArray ba2("hello");
    ba2.insert(0, "pre", 3);
    REQUIRE_EQ(ba2, ByteArray("prehello"));

    // insert const char*
    ByteArray ba3("world");
    ba3.insert(0, "hello ");
    REQUIRE_EQ(ba3, ByteArray("hello world"));
}

// Mid/Left/Right tests
TEST_CASE("ByteArray: mid_left_right")
{
    ByteArray ba("hello world");

    // mid
    REQUIRE_EQ(ba.mid(0), ByteArray("hello world"));
    REQUIRE_EQ(ba.mid(6), ByteArray("world"));
    REQUIRE_EQ(ba.mid(0, 5), ByteArray("hello"));
    REQUIRE_EQ(ba.mid(6, 5), ByteArray("world"));
    REQUIRE_EQ(ba.mid(11), ByteArray(""));
    REQUIRE_EQ(ba.mid(20), ByteArray(""));

    // left
    REQUIRE_EQ(ba.left(5), ByteArray("hello"));
    REQUIRE_EQ(ba.left(11), ByteArray("hello world"));
    REQUIRE_EQ(ba.left(20), ByteArray("hello world"));

    // right
    REQUIRE_EQ(ba.right(5), ByteArray("world"));
    REQUIRE_EQ(ba.right(11), ByteArray("hello world"));
    REQUIRE_EQ(ba.right(20), ByteArray("hello world"));
}

// StartsWith/EndsWith tests
TEST_CASE("ByteArray: startsWith_endsWith")
{
    ByteArray ba("hello world");

    REQUIRE(ba.startsWith("hello"));
    REQUIRE(ba.startsWith("hello world"));
    REQUIRE(!ba.startsWith("world"));
    REQUIRE(!ba.startsWith("HELLO"));

    // From ByteArray
    REQUIRE(ba.startsWith(ByteArray("hello")));
    REQUIRE(ba.endsWith(ByteArray("world")));

    REQUIRE(ba.endsWith("world"));
    REQUIRE(ba.endsWith("hello world"));
    REQUIRE(!ba.endsWith("hello"));
    REQUIRE(!ba.endsWith("WORLD"));
}

// Contains tests
TEST_CASE("ByteArray: contains")
{
    ByteArray ba("hello world");

    REQUIRE(ba.contains("hello"));
    REQUIRE(ba.contains("world"));
    REQUIRE(ba.contains("lo wo"));
    REQUIRE(!ba.contains("xyz"));
    REQUIRE(!ba.contains("HELLO"));

    REQUIRE(ba.contains('h'));
    REQUIRE(ba.contains('w'));
    REQUIRE(!ba.contains('z'));
    REQUIRE(!ba.contains('H'));  // case sensitive
}

// IndexOf tests
TEST_CASE("ByteArray: indexOf")
{
    ByteArray ba("hello world hello");

    // From char
    REQUIRE_EQ(ba.indexOf('h'), (ssize_t)0);
    REQUIRE_EQ(ba.indexOf('w'), (ssize_t)6);
    REQUIRE_EQ(ba.indexOf('z'), (ssize_t)-1);

    // From const char*
    REQUIRE_EQ(ba.indexOf("hello"), (ssize_t)0);
    REQUIRE_EQ(ba.indexOf("world"), (ssize_t)6);
    REQUIRE_EQ(ba.indexOf("xyz"), (ssize_t)-1);

    // From ByteArray
    REQUIRE_EQ(ba.indexOf(ByteArray("hello")), (ssize_t)0);
    REQUIRE_EQ(ba.indexOf(ByteArray("world")), (ssize_t)6);

    // With from offset
    REQUIRE_EQ(ba.indexOf("hello", 1), (ssize_t)12);
    REQUIRE(ba.indexOf('l', 3) == 3);
    REQUIRE(ba.indexOf('l', 4) == 9);
}

// Replace tests
TEST_CASE("ByteArray: replace")
{
    ByteArray ba("hello world hello");

    // replace(const char*, const char*)
    ba.replace("hello", "hi");
    REQUIRE_EQ(ba, ByteArray("hi world hi"));

    // replace(char, const char*)
    ByteArray ba2("test");
    ba2.replace('t', "x");
    REQUIRE_EQ(ba2, ByteArray("xesx"));

    // replace(size_t, size_t, const char*, size_t)
    ByteArray ba3("abcdef");
    ba3.replace(1, 3, "xyz", 3);
    REQUIRE_EQ(ba3, ByteArray("axyzef"));

    // replace(size_t, size_t, const char*)
    ByteArray ba4("abcdef");
    ba4.replace(1, 3, "zzz");
    REQUIRE_EQ(ba4, ByteArray("azzzef"));
}

// Self-modification: the source aliases the block's own buffer. Before the
// safeSource() guard, grow() would realloc the block (freeing the old buffer)
// and the subsequent copy would read freed memory (use-after-free). These force
// a grow so the regression is covered under ASan.
TEST_CASE("ByteArray: self_modification")
{
    // self-append doubles the content; 300 -> 600 > capacity 512 forces grow()
    ByteArray a(300, 'x');
    a += a;
    REQUIRE_EQ(a.size(), (size_t)600);
    REQUIRE(a.startsWith("xxxx"));
    REQUIRE_EQ(a[599], 'x');

    // self-insert at the front
    ByteArray b("abc");
    b.insert(0, b);
    REQUIRE_EQ(b, ByteArray("abcabc"));

    // self-replace (insert the whole content at pos 0)
    ByteArray c("abcd");
    c.replace(0, 0, c.constData(), 4);
    REQUIRE_EQ(c, ByteArray("abcdabcd"));
}

// Trimmed test
TEST_CASE("ByteArray: trimmed")
{
    REQUIRE_EQ(ByteArray("  hello  ").trimmed(), ByteArray("hello"));
    REQUIRE_EQ(ByteArray("\t\nhello\r\n  ").trimmed(), ByteArray("hello"));
    REQUIRE_EQ(ByteArray("  \t  ").trimmed(), ByteArray(""));
    REQUIRE_EQ(ByteArray(" hello world ").trimmed(), ByteArray("hello world"));
}

// Simplified test
TEST_CASE("ByteArray: simplified")
{
    REQUIRE_EQ(ByteArray("  hello  ").simplified(), ByteArray("hello"));
    REQUIRE_EQ(ByteArray("  hello   world  ").simplified(), ByteArray("hello world"));
    REQUIRE_EQ(ByteArray("\t\nhello\r\n  world\t").simplified(), ByteArray("hello world"));
    REQUIRE_EQ(ByteArray("  \t  ").simplified(), ByteArray(""));
    REQUIRE_EQ(ByteArray("hello").simplified(), ByteArray("hello"));
}

// Base64 tests
TEST_CASE("ByteArray: toBase64_fromBase64")
{
    // Empty
    REQUIRE_EQ(ByteArray().toBase64(), ByteArray(""));

    // Basic
    REQUIRE_EQ(ByteArray("A").toBase64(), ByteArray("QQ=="));
    REQUIRE_EQ(ByteArray("AB").toBase64(), ByteArray("QUI="));
    REQUIRE_EQ(ByteArray("ABC").toBase64(), ByteArray("QUJD"));

    // Roundtrip
    ByteArray original("Hello World!");
    ByteArray encoded = original.toBase64();
    ByteArray decoded = ByteArray::fromBase64(encoded);
    REQUIRE_EQ(decoded, original);
}

// Hex tests
TEST_CASE("ByteArray: toHex_fromHex")
{
    // Empty
    REQUIRE_EQ(ByteArray().toHex(), ByteArray(""));

    // Basic
    REQUIRE(ByteArray("A").toHex() == ByteArray("41"));
    REQUIRE(ByteArray("\xFF").toHex() == ByteArray("ff"));
    // Use ByteArray constructor with explicit length for binary data
    REQUIRE(ByteArray("\x00\xFF\x01", 3).toHex() == ByteArray("00ff01"));

    // fromHex
    REQUIRE(ByteArray::fromHex("41") == ByteArray("A"));
    REQUIRE(ByteArray::fromHex("4142") == ByteArray("AB"));
    REQUIRE(ByteArray::fromHex("00ff01") == ByteArray("\x00\xFF\x01", 3));

    // fromHex ignores whitespace
    REQUIRE(ByteArray::fromHex("41 42") == ByteArray("AB"));
    REQUIRE(ByteArray::fromHex("41\n42") == ByteArray("AB"));
}

// Numeric conversions tests
TEST_CASE("ByteArray: numeric_conversions")
{
    bool ok;

    // toUInt
    REQUIRE_EQ(ByteArray("42").toUInt(&ok), (uint32)42);
    REQUIRE(ok);
    REQUIRE_EQ(ByteArray("0").toUInt(&ok), (uint32)0);
    REQUIRE(ok);
    REQUIRE_EQ(ByteArray("abc").toUInt(&ok), (uint32)0);
    REQUIRE(!ok);

    // toInt
    REQUIRE_EQ(ByteArray("-42").toInt(&ok), (int32)-42);
    REQUIRE(ok);
    REQUIRE_EQ(ByteArray("abc").toInt(&ok), (int32)0);
    REQUIRE(!ok);

    // toULong
    REQUIRE_EQ(ByteArray("42").toULong(&ok), (uint64)42);
    REQUIRE(ok);
    REQUIRE_EQ(ByteArray("123456789012345").toULong(&ok), (uint64)123456789012345LL);
    REQUIRE(ok);
    REQUIRE_EQ(ByteArray("abc").toULong(&ok), (uint64)0);
    REQUIRE(!ok);

    // toLong
    REQUIRE_EQ(ByteArray("42").toLong(&ok), (int64)42);
    REQUIRE(ok);
    REQUIRE_EQ(ByteArray("-42").toLong(&ok), (int64)-42);
    REQUIRE(ok);
    REQUIRE_EQ(ByteArray("").toLong(&ok), (int64)0);
    REQUIRE(!ok);
    REQUIRE_EQ(ByteArray("abc").toLong(&ok), (int64)0);
    REQUIRE(!ok);
    REQUIRE_EQ(ByteArray("123abc").toLong(&ok), (int64)123);
    REQUIRE(!ok);  // trailing characters
    REQUIRE_EQ(ByteArray("42 ").toLong(&ok), (int64)42);
    REQUIRE(!ok);  // trailing space means not fully consumed
    REQUIRE_EQ(ByteArray("9223372036854775807").toLong(&ok), (int64)9223372036854775807LL);
    REQUIRE(ok);
    REQUIRE_EQ(ByteArray("-9223372036854775807").toLong(&ok), (int64)-9223372036854775807LL);
    REQUIRE(ok);
}

// Number formatting tests
TEST_CASE("ByteArray: number")
{
    REQUIRE_EQ(ByteArray::number(42), ByteArray("42"));
    REQUIRE_EQ(ByteArray::number(-42), ByteArray("-42"));
    REQUIRE_EQ(ByteArray::number(9223372036854775807LL), ByteArray("9223372036854775807"));
    REQUIRE_EQ(ByteArray::number(-9223372036854775807LL), ByteArray("-9223372036854775807"));
    REQUIRE(ByteArray::number(3.14159).indexOf("3.14") == 0);
    REQUIRE(ByteArray::number(2.71828).indexOf("2.71") == 0);
}

// Split test
TEST_CASE("ByteArray: split")
{
    ByteArray ba("a,b,c,d");

    // Split by char
    auto parts1 = ba.split(',');
    REQUIRE_EQ(parts1.size(), (size_t)4);
    REQUIRE_EQ(parts1[0], ByteArray("a"));
    REQUIRE_EQ(parts1[1], ByteArray("b"));
    REQUIRE_EQ(parts1[2], ByteArray("c"));
    REQUIRE_EQ(parts1[3], ByteArray("d"));

    // Empty string split
    auto parts2 = ByteArray("").split(',');
    REQUIRE_EQ(parts2.size(), (size_t)1);
    REQUIRE_EQ(parts2[0], ByteArray(""));

    // Single element without delimiter
    auto parts3 = ByteArray("abc").split(',');
    REQUIRE_EQ(parts3.size(), (size_t)1);
    REQUIRE_EQ(parts3[0], ByteArray("abc"));

    // Consecutive delimiters
    auto parts4 = ByteArray("a,,b").split(',');
    REQUIRE_EQ(parts4.size(), (size_t)3);
    REQUIRE_EQ(parts4[0], ByteArray("a"));
    REQUIRE_EQ(parts4[1], ByteArray(""));
    REQUIRE_EQ(parts4[2], ByteArray("b"));
}

// Concatenation tests
TEST_CASE("ByteArray: concatenation")
{
    ByteArray ba1("hello");
    ByteArray ba2(" world");

    // operator+
    REQUIRE_EQ(ba1 + ba2, ByteArray("hello world"));
    REQUIRE_EQ(ba1 + " world", ByteArray("hello world"));
    REQUIRE_EQ("hello" + ba2, ByteArray("hello world"));

    // operator+=
    ByteArray ba3("hello");
    ba3 += " world";
    REQUIRE_EQ(ba3, ByteArray("hello world"));
}

// Implicit sharing tests
TEST_CASE("ByteArray: implicit_sharing")
{
    ByteArray ba1("hello");
    ByteArray ba2 = ba1;

    // Initially share (same data)
    REQUIRE_EQ(ba1, ba2);

    // Verify const pointers point to same address while sharing
    const char * data1 = ba1.constData();
    const char * data2 = ba2.constData();
    REQUIRE(data1 == data2);

    // Modify ba2 - should detach
    ba2[0] = 'x';
    REQUIRE_EQ(ba1, ByteArray("hello"));
    REQUIRE_EQ(ba2, ByteArray("xello"));

    // ba1 should not be affected by ba2's modification
    REQUIRE(ba1 != ba2);

    // Verify const pointers now point to different addresses after detach
    REQUIRE(ba1.constData() != ba2.constData());

    // Test detach on shared data
    ByteArray ba3("test");
    ByteArray ba4 = ba3;
    ba3.detach();
    ba3[0] = 'x';
    REQUIRE_EQ(ba3, ByteArray("xest"));
    REQUIRE_EQ(ba4, ByteArray("test"));

    // Verify const pointers are different after explicit detach
    REQUIRE(ba3.constData() != ba4.constData());
}

// char* on the left side of comparisons (free function, direct body)
TEST_CASE("ByteArray: char_ptr_comparison")
{
    ByteArray ba("hello");
    REQUIRE("hello" == ba);
    REQUIRE("world" != ba);
    REQUIRE(ba == "hello");
    REQUIRE(ba != "world");

    const char * cp = nullptr;
    ByteArray nullBa;
    REQUIRE(cp == nullBa);
    REQUIRE(cp != ba);
    REQUIRE(nullBa == cp);
    REQUIRE(ba != cp);
}

// Move-assignment into a ByteArray that already holds a block must
// release the old block (previously leaked; verified under ASan/LSan)
TEST_CASE("ByteArray: move_semantics")
{
    ByteArray a("hello");
    ByteArray b("world");

    ByteArray c = std::move(b);
    REQUIRE_EQ(c, ByteArray("world"));
    REQUIRE(b.isNull());

    a = std::move(c);
    REQUIRE_EQ(a, ByteArray("world"));
    REQUIRE(c.isNull());

    // self move-assign is a no-op (through an alias so the test itself
    // does not trigger -Wself-move)
    ByteArray & self = a;
    a = std::move(self);
    REQUIRE_EQ(a, ByteArray("world"));

    // shared block: move-assign releases the shared old block correctly
    ByteArray x("shared");
    ByteArray y = x;           // x, y share
    ByteArray z("other");
    x = std::move(z);          // x releases "shared"
    REQUIRE_EQ(x, ByteArray("other"));
    REQUIRE_EQ(y, ByteArray("shared"));
    REQUIRE(z.isNull());

    // String: the `s = s.left(n)` pattern (substring assigned to self)
    String s("abcdefgh");
    s = s.left(3);
    REQUIRE_EQ(s, String("abc"));
    s = s.left(1);
    REQUIRE_EQ(s, String("a"));
}

}

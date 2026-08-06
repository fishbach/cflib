/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/util/test.h>
#include <cflib/util/tuplecompare.h>
#include <cflib/util/util.h>

#include <format>
#include <iostream>

using namespace cflib::util;

TEST_SUITE("util") {

TEST_CASE("util: flatten")
{
    REQUIRE_EQ(flatten(String()), String());
    REQUIRE_EQ(flatten(""), String(""));
    REQUIRE_EQ(flatten("      \r\n"), String(""));
    REQUIRE_EQ(flatten("     ab_c & 1.2-3\r\n"), String("ab_c_1.2-3"));
    REQUIRE_EQ(flatten("     _ \r\n_"), String("_"));
}

TEST_CASE("util: String")
{
    REQUIRE(String().isNull());
    REQUIRE(!String("").isNull());
}

TEST_CASE("util: ByteArray")
{
    ByteArray ba;
    REQUIRE(ba.isNull());
    ba.reserve(997);
    REQUIRE(ba.capacity() >= (size_t)997);
    ba.resize(123);
    REQUIRE_EQ(ba.size(), (size_t)123);
    REQUIRE(ba.capacity() >= (size_t)997);
    ba.resize(0);
    REQUIRE(!ba.isNull());
    REQUIRE_EQ(ba.size(), (size_t)0);
    REQUIRE(ba.capacity() >= (size_t)997);
    ba.clear();
    REQUIRE(ba.isNull());
    REQUIRE(!ByteArray("").isNull());
}

TEST_CASE("util: deflate")
{
    auto checkDeflate = [](const ByteArray & source, const ByteArray & enc, int level) -> bool {
        ByteArray data = source;
        deflateRaw(data, level);
        if (data != ByteArray::fromHex(enc)) {
            std::cout << std::format("compressed differs:\nis: {}\nex: {}\n",
                data.toHex().constData(), enc.constData());
            return false;
        }
        inflateRaw(data);
        if (data != source) {
            std::cout << std::format("decompressed differs:\nis: {}\nex: {}\n",
                data.toHex().constData(), source.toHex().constData());
            return false;
        }
        return true;
    };

    REQUIRE(checkDeflate(ByteArray(),                 "00",                     1));
    REQUIRE(checkDeflate(ByteArray(),                 "00",                     0));
    REQUIRE(checkDeflate(ByteArray("\0", 1),          "620000",                 1));
    REQUIRE(checkDeflate(ByteArray("A"),              "720400",                 1));
    REQUIRE(checkDeflate(ByteArray("A"),              "000100feff4100",         0));
    REQUIRE(checkDeflate(ByteArray("bc"),             "4a4a0600",               1));
    REQUIRE(checkDeflate(ByteArray("Hello"),          "f248cdc9c90700",         1));
    REQUIRE(checkDeflate(ByteArray("Hello"),          "000500faff48656c6c6f00", 0));
    ByteArray data;
    inflateRaw(data);
    REQUIRE(data.isEmpty());
    data = "\x00";
    inflateRaw(data);
    REQUIRE(data.isEmpty());
}

TEST_CASE("util: tupleCompare")
{
    REQUIRE( equal(std::tuple<int, float>(2, 3.14f), 2, 3.14f));
    REQUIRE(!equal(std::tuple<int, float>(2, 3.14f), 2, 3.2f));
    REQUIRE( equal(std::tuple<int, float>(2, 3.14f), 2));
    REQUIRE(!equal(std::tuple<int, float>(2, 3.14f), 3));
    REQUIRE( equal(std::tuple<int, float>(2, 3.14f)));

    REQUIRE( partialEqual(std::tuple<int, float>(2, 3.14f), 2, 2, 3.14f));
    REQUIRE(!partialEqual(std::tuple<int, float>(2, 3.14f), 2, 2, 3.2f));
    REQUIRE(!partialEqual(std::tuple<int, float>(2, 3.14f), 2, 3, 3.14f));
    REQUIRE( partialEqual(std::tuple<int, float>(2, 3.14f), 2, 2));
    REQUIRE(!partialEqual(std::tuple<int, float>(2, 3.14f), 2, 3));
    REQUIRE( partialEqual(std::tuple<int, float>(2, 3.14f), 2));

    REQUIRE( partialEqual(std::tuple<int, float>(2, 3.14f), 1, 2, 3.14f));
    REQUIRE( partialEqual(std::tuple<int, float>(2, 3.14f), 1, 2, 3.2f));
    REQUIRE(!partialEqual(std::tuple<int, float>(2, 3.14f), 1, 3, 3.14f));
    REQUIRE( partialEqual(std::tuple<int, float>(2, 3.14f), 1, 2));
    REQUIRE(!partialEqual(std::tuple<int, float>(2, 3.14f), 1, 3));
    REQUIRE( partialEqual(std::tuple<int, float>(2, 3.14f), 1));

    REQUIRE( partialEqual(std::tuple<int, float>(2, 3.14f), 0, 3, 3.2f));
    REQUIRE( partialEqual(std::tuple<int, float>(2, 3.14f), 0));
}

TEST_CASE("util: callWithTupleParams")
{
    int i = 0;
    float f = 0.0;
    callWithTupleParams<void>([&](int pi, float pf) { i = pi; f = pf; }, std::tuple<int, float>(2, 3.14f));
    REQUIRE_EQ(i, 2);
    REQUIRE_EQ(f, 3.14f);
    callWithTupleParams<void>([&](int & i, float pf) { ++i; f = pf; }, std::tuple<float>(2.34f), i);
    REQUIRE_EQ(i, 3);
    REQUIRE_EQ(f, 2.34f);
}

}

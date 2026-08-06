/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/serialize/util.h>
#include <cflib/util/test.h>

using namespace cflib::serialize;

TEST_SUITE("serialize") {

TEST_CASE("serialize: getTLVLength")
{
    uint64 tag;
    int tagLen;
    int lengthSize;
    REQUIRE_EQ(getTLVLength(ByteArray::fromHex(""            ), tag, tagLen, lengthSize), -1);
    REQUIRE_EQ(getTLVLength(ByteArray::fromHex("c1"          ), tag, tagLen, lengthSize), -1);
    REQUIRE_EQ(getTLVLength(ByteArray::fromHex("c100"        ), tag, tagLen, lengthSize),  0); REQUIRE_EQ(lengthSize, 1);
    REQUIRE_EQ(getTLVLength(ByteArray::fromHex("c18100"      ), tag, tagLen, lengthSize),  0); REQUIRE_EQ(lengthSize, 2);
    REQUIRE_EQ(getTLVLength(ByteArray::fromHex("c180"        ), tag, tagLen, lengthSize), -2);
    REQUIRE_EQ(getTLVLength(ByteArray::fromHex("c101"        ), tag, tagLen, lengthSize), -1);
    REQUIRE_EQ(getTLVLength(ByteArray::fromHex("c10100"      ), tag, tagLen, lengthSize),  1); REQUIRE_EQ(lengthSize, 1);
    REQUIRE_EQ(getTLVLength(ByteArray::fromHex("c1810100"    ), tag, tagLen, lengthSize),  1); REQUIRE_EQ(lengthSize, 2);
    REQUIRE_EQ(getTLVLength(ByteArray::fromHex("c1847FFFFFF9"), tag, tagLen, lengthSize), -1);
    REQUIRE_EQ(getTLVLength(ByteArray::fromHex("c1847FFFFFFA"), tag, tagLen, lengthSize), -3);
    REQUIRE_EQ(getTLVLength(ByteArray::fromHex("c18480000000"), tag, tagLen, lengthSize), -3);
    REQUIRE_EQ(getTLVLength(ByteArray::fromHex("c188"        ), tag, tagLen, lengthSize), -1);
    REQUIRE_EQ(getTLVLength(ByteArray::fromHex("c189"        ), tag, tagLen, lengthSize), -3);

    getTLVLength(ByteArray::fromHex("c000"    ), tag, tagLen, lengthSize); REQUIRE_EQ(tag, (uint64)  0); REQUIRE_EQ(tagLen, 1);
    getTLVLength(ByteArray::fromHex("c100"    ), tag, tagLen, lengthSize); REQUIRE_EQ(tag, (uint64)  1); REQUIRE_EQ(tagLen, 1);
    getTLVLength(ByteArray::fromHex("DE00"    ), tag, tagLen, lengthSize); REQUIRE_EQ(tag, (uint64) 30); REQUIRE_EQ(tagLen, 1);
    getTLVLength(ByteArray::fromHex("DF1F00"  ), tag, tagLen, lengthSize); REQUIRE_EQ(tag, (uint64) 31); REQUIRE_EQ(tagLen, 2);
    getTLVLength(ByteArray::fromHex("DF7F00"  ), tag, tagLen, lengthSize); REQUIRE_EQ(tag, (uint64)127); REQUIRE_EQ(tagLen, 2);
    getTLVLength(ByteArray::fromHex("DF810000"), tag, tagLen, lengthSize); REQUIRE_EQ(tag, (uint64)128); REQUIRE_EQ(tagLen, 3);
    getTLVLength(ByteArray::fromHex("DF810100"), tag, tagLen, lengthSize); REQUIRE_EQ(tag, (uint64)129); REQUIRE_EQ(tagLen, 3);
}

TEST_CASE("serialize: toByteArray")
{
    REQUIRE_EQ(toByteArray(0, 0), ByteArray::fromHex("C08100"));
    REQUIRE_EQ(toByteArray(0), ByteArray::fromHex(""));

    REQUIRE_EQ(toByteArray(1), ByteArray::fromHex("C10101"));
    REQUIRE_EQ(toByteArray(-1), ByteArray::fromHex("C101FF"));
    REQUIRE_EQ(toByteArray("bla"), ByteArray::fromHex("C103626C61"));

    REQUIRE_EQ(toByteArray(1,    30), ByteArray::fromHex("DE0101"));
    REQUIRE_EQ(toByteArray(1,    31), ByteArray::fromHex("DF1F0101"));
    REQUIRE_EQ(toByteArray(1,   127), ByteArray::fromHex("DF7F0101"));
    REQUIRE_EQ(toByteArray(1,   128), ByteArray::fromHex("DF81000101"));
    REQUIRE_EQ(toByteArray(1,   129), ByteArray::fromHex("DF81010101"));
    REQUIRE_EQ(toByteArray(1,   255), ByteArray::fromHex("DF817F0101"));
    REQUIRE_EQ(toByteArray(1,   256), ByteArray::fromHex("DF82000101"));
    REQUIRE_EQ(toByteArray(1, 16383), ByteArray::fromHex("DFFF7F0101"));
    REQUIRE_EQ(toByteArray(1, 16384), ByteArray::fromHex("DF8180000101"));

    REQUIRE_EQ(toByteArray(ByteArray(),   3), ByteArray::fromHex(""));
    REQUIRE_EQ(toByteArray(ByteArray(""), 3), ByteArray::fromHex("C300"));
}

TEST_CASE("serialize: fromByteArray")
{
    REQUIRE_EQ(fromByteArray<int>(ByteArray::fromHex("")), 0);
    REQUIRE_EQ(fromByteArray<int>(ByteArray::fromHex("C000")), 0);
    REQUIRE_EQ(fromByteArray<int>(ByteArray::fromHex("C08100")), 0);
    REQUIRE_EQ(fromByteArray<int>(ByteArray::fromHex("C10101")), 1);
    REQUIRE_EQ(fromByteArray<int>(ByteArray::fromHex("C101FF")), -1);
    REQUIRE_EQ(fromByteArray<String>(ByteArray::fromHex("")), String());
    REQUIRE_EQ(fromByteArray<String>(ByteArray::fromHex("C000")), String());
    REQUIRE(!fromByteArray<String>(ByteArray::fromHex("C000")).isNull());
    REQUIRE_EQ(fromByteArray<String>(ByteArray::fromHex("C08100")), String());
    REQUIRE(fromByteArray<String>(ByteArray::fromHex("C08100")).isNull());
    REQUIRE_EQ(fromByteArray<String>(ByteArray::fromHex("C103626C61")), String("bla"));
}

TEST_CASE("serialize: sizes")
{
    REQUIRE_EQ((int)sizeof(float      ),  4);
    REQUIRE_EQ((int)sizeof(double     ),  8);
    REQUIRE_EQ((int)sizeof(long double), 16);
}

TEST_CASE("serialize: readAndCall")
{
    BERSerializer ser;
    ser << 34 << "bla";
    BERDeserializer deser(ser.data());
    int i = 0;
    String s;
    readAndCall<int, const String &>(deser, [&](int pi, const String & ps) { i = pi; s = ps; });
    REQUIRE_EQ(i, 34);
    REQUIRE_EQ(s, String("bla"));
}

}

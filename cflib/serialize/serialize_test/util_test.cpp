/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/serialize/util.h>
#include <cflib/util/test.h>

using namespace cflib::serialize;

class Util_Test : public cflib::util::TestBase
{
public:
    std::vector<cflib::util::TestMethod> testMethods() const override {
        auto self = const_cast<Util_Test *>(this);
        return {
            {"test_getTLVLength",  [self]() { self->test_getTLVLength(); }},
            {"test_toByteArray",   [self]() { self->test_toByteArray(); }},
            {"test_fromByteArray", [self]() { self->test_fromByteArray(); }},
            {"test_sizes",         [self]() { self->test_sizes(); }},
            {"test_readAndCall",   [self]() { self->test_readAndCall(); }}
        };
    }

    void test_getTLVLength()
    {
        cfuint64 tag;
        int tagLen;
        int lengthSize;
        QCOMPARE(getTLVLength(CFByteArray::fromHex(""            ), tag, tagLen, lengthSize), -1);
        QCOMPARE(getTLVLength(CFByteArray::fromHex("c1"          ), tag, tagLen, lengthSize), -1);
        QCOMPARE(getTLVLength(CFByteArray::fromHex("c100"        ), tag, tagLen, lengthSize),  0); QCOMPARE(lengthSize, 1);
        QCOMPARE(getTLVLength(CFByteArray::fromHex("c18100"      ), tag, tagLen, lengthSize),  0); QCOMPARE(lengthSize, 2);
        QCOMPARE(getTLVLength(CFByteArray::fromHex("c180"        ), tag, tagLen, lengthSize), -2);
        QCOMPARE(getTLVLength(CFByteArray::fromHex("c101"        ), tag, tagLen, lengthSize), -1);
        QCOMPARE(getTLVLength(CFByteArray::fromHex("c10100"      ), tag, tagLen, lengthSize),  1); QCOMPARE(lengthSize, 1);
        QCOMPARE(getTLVLength(CFByteArray::fromHex("c1810100"    ), tag, tagLen, lengthSize),  1); QCOMPARE(lengthSize, 2);
        QCOMPARE(getTLVLength(CFByteArray::fromHex("c1847FFFFFF9"), tag, tagLen, lengthSize), -1);
        QCOMPARE(getTLVLength(CFByteArray::fromHex("c1847FFFFFFA"), tag, tagLen, lengthSize), -3);
        QCOMPARE(getTLVLength(CFByteArray::fromHex("c18480000000"), tag, tagLen, lengthSize), -3);
        QCOMPARE(getTLVLength(CFByteArray::fromHex("c188"        ), tag, tagLen, lengthSize), -1);
        QCOMPARE(getTLVLength(CFByteArray::fromHex("c189"        ), tag, tagLen, lengthSize), -3);

        getTLVLength(CFByteArray::fromHex("c000"    ), tag, tagLen, lengthSize); QCOMPARE(tag, (cfuint64)  0); QCOMPARE(tagLen, 1);
        getTLVLength(CFByteArray::fromHex("c100"    ), tag, tagLen, lengthSize); QCOMPARE(tag, (cfuint64)  1); QCOMPARE(tagLen, 1);
        getTLVLength(CFByteArray::fromHex("DE00"    ), tag, tagLen, lengthSize); QCOMPARE(tag, (cfuint64) 30); QCOMPARE(tagLen, 1);
        getTLVLength(CFByteArray::fromHex("DF1F00"  ), tag, tagLen, lengthSize); QCOMPARE(tag, (cfuint64) 31); QCOMPARE(tagLen, 2);
        getTLVLength(CFByteArray::fromHex("DF7F00"  ), tag, tagLen, lengthSize); QCOMPARE(tag, (cfuint64)127); QCOMPARE(tagLen, 2);
        getTLVLength(CFByteArray::fromHex("DF810000"), tag, tagLen, lengthSize); QCOMPARE(tag, (cfuint64)128); QCOMPARE(tagLen, 3);
        getTLVLength(CFByteArray::fromHex("DF810100"), tag, tagLen, lengthSize); QCOMPARE(tag, (cfuint64)129); QCOMPARE(tagLen, 3);
    }

    void test_toByteArray()
    {
        QCOMPARE(toByteArray(0, 0), CFByteArray::fromHex("C08100"));
        QCOMPARE(toByteArray(0), CFByteArray::fromHex(""));

        QCOMPARE(toByteArray(1), CFByteArray::fromHex("C10101"));
        QCOMPARE(toByteArray(-1), CFByteArray::fromHex("C101FF"));
        QCOMPARE(toByteArray("bla"), CFByteArray::fromHex("C103626C61"));

        QCOMPARE(toByteArray(1,    30), CFByteArray::fromHex("DE0101"));
        QCOMPARE(toByteArray(1,    31), CFByteArray::fromHex("DF1F0101"));
        QCOMPARE(toByteArray(1,   127), CFByteArray::fromHex("DF7F0101"));
        QCOMPARE(toByteArray(1,   128), CFByteArray::fromHex("DF81000101"));
        QCOMPARE(toByteArray(1,   129), CFByteArray::fromHex("DF81010101"));
        QCOMPARE(toByteArray(1,   255), CFByteArray::fromHex("DF817F0101"));
        QCOMPARE(toByteArray(1,   256), CFByteArray::fromHex("DF82000101"));
        QCOMPARE(toByteArray(1, 16383), CFByteArray::fromHex("DFFF7F0101"));
        QCOMPARE(toByteArray(1, 16384), CFByteArray::fromHex("DF8180000101"));

        QCOMPARE(toByteArray(CFByteArray(),   3), CFByteArray::fromHex(""));
        QCOMPARE(toByteArray(CFByteArray(""), 3), CFByteArray::fromHex("C300"));
    }

    void test_fromByteArray()
    {
        QCOMPARE(fromByteArray<int>(CFByteArray::fromHex("")), 0);
        QCOMPARE(fromByteArray<int>(CFByteArray::fromHex("C000")), 0);
        QCOMPARE(fromByteArray<int>(CFByteArray::fromHex("C08100")), 0);
        QCOMPARE(fromByteArray<int>(CFByteArray::fromHex("C10101")), 1);
        QCOMPARE(fromByteArray<int>(CFByteArray::fromHex("C101FF")), -1);
        QCOMPARE(fromByteArray<CFString>(CFByteArray::fromHex("")), CFString());
        QCOMPARE(fromByteArray<CFString>(CFByteArray::fromHex("C000")), CFString());
        QVERIFY(!fromByteArray<CFString>(CFByteArray::fromHex("C000")).isNull());
        QCOMPARE(fromByteArray<CFString>(CFByteArray::fromHex("C08100")), CFString());
        QVERIFY(fromByteArray<CFString>(CFByteArray::fromHex("C08100")).isNull());
        QCOMPARE(fromByteArray<CFString>(CFByteArray::fromHex("C103626C61")), CFString("bla"));
    }

    void test_sizes()
    {
        QCOMPARE((int)sizeof(float      ),  4);
        QCOMPARE((int)sizeof(double     ),  8);
        QCOMPARE((int)sizeof(long double), 16);
    }

    void test_readAndCall()
    {
        BERSerializer ser;
        ser << 34 << "bla";
        BERDeserializer deser(ser.data());
        int i = 0;
        CFString s;
        readAndCall<int, const CFString &>(deser, [&](int pi, const CFString & ps) { i = pi; s = ps; });
        QCOMPARE(i, 34);
        QCOMPARE(s, CFString("bla"));
    }
};

ADD_TEST(Util_Test)

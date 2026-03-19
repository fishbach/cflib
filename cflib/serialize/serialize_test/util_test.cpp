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
        TCOMPARE(getTLVLength(CFByteArray::fromHex(""            ), tag, tagLen, lengthSize), -1);
        TCOMPARE(getTLVLength(CFByteArray::fromHex("c1"          ), tag, tagLen, lengthSize), -1);
        TCOMPARE(getTLVLength(CFByteArray::fromHex("c100"        ), tag, tagLen, lengthSize),  0); TCOMPARE(lengthSize, 1);
        TCOMPARE(getTLVLength(CFByteArray::fromHex("c18100"      ), tag, tagLen, lengthSize),  0); TCOMPARE(lengthSize, 2);
        TCOMPARE(getTLVLength(CFByteArray::fromHex("c180"        ), tag, tagLen, lengthSize), -2);
        TCOMPARE(getTLVLength(CFByteArray::fromHex("c101"        ), tag, tagLen, lengthSize), -1);
        TCOMPARE(getTLVLength(CFByteArray::fromHex("c10100"      ), tag, tagLen, lengthSize),  1); TCOMPARE(lengthSize, 1);
        TCOMPARE(getTLVLength(CFByteArray::fromHex("c1810100"    ), tag, tagLen, lengthSize),  1); TCOMPARE(lengthSize, 2);
        TCOMPARE(getTLVLength(CFByteArray::fromHex("c1847FFFFFF9"), tag, tagLen, lengthSize), -1);
        TCOMPARE(getTLVLength(CFByteArray::fromHex("c1847FFFFFFA"), tag, tagLen, lengthSize), -3);
        TCOMPARE(getTLVLength(CFByteArray::fromHex("c18480000000"), tag, tagLen, lengthSize), -3);
        TCOMPARE(getTLVLength(CFByteArray::fromHex("c188"        ), tag, tagLen, lengthSize), -1);
        TCOMPARE(getTLVLength(CFByteArray::fromHex("c189"        ), tag, tagLen, lengthSize), -3);

        getTLVLength(CFByteArray::fromHex("c000"    ), tag, tagLen, lengthSize); TCOMPARE(tag, (cfuint64)  0); TCOMPARE(tagLen, 1);
        getTLVLength(CFByteArray::fromHex("c100"    ), tag, tagLen, lengthSize); TCOMPARE(tag, (cfuint64)  1); TCOMPARE(tagLen, 1);
        getTLVLength(CFByteArray::fromHex("DE00"    ), tag, tagLen, lengthSize); TCOMPARE(tag, (cfuint64) 30); TCOMPARE(tagLen, 1);
        getTLVLength(CFByteArray::fromHex("DF1F00"  ), tag, tagLen, lengthSize); TCOMPARE(tag, (cfuint64) 31); TCOMPARE(tagLen, 2);
        getTLVLength(CFByteArray::fromHex("DF7F00"  ), tag, tagLen, lengthSize); TCOMPARE(tag, (cfuint64)127); TCOMPARE(tagLen, 2);
        getTLVLength(CFByteArray::fromHex("DF810000"), tag, tagLen, lengthSize); TCOMPARE(tag, (cfuint64)128); TCOMPARE(tagLen, 3);
        getTLVLength(CFByteArray::fromHex("DF810100"), tag, tagLen, lengthSize); TCOMPARE(tag, (cfuint64)129); TCOMPARE(tagLen, 3);
    }

    void test_toByteArray()
    {
        TCOMPARE(toByteArray(0, 0), CFByteArray::fromHex("C08100"));
        TCOMPARE(toByteArray(0), CFByteArray::fromHex(""));

        TCOMPARE(toByteArray(1), CFByteArray::fromHex("C10101"));
        TCOMPARE(toByteArray(-1), CFByteArray::fromHex("C101FF"));
        TCOMPARE(toByteArray("bla"), CFByteArray::fromHex("C103626C61"));

        TCOMPARE(toByteArray(1,    30), CFByteArray::fromHex("DE0101"));
        TCOMPARE(toByteArray(1,    31), CFByteArray::fromHex("DF1F0101"));
        TCOMPARE(toByteArray(1,   127), CFByteArray::fromHex("DF7F0101"));
        TCOMPARE(toByteArray(1,   128), CFByteArray::fromHex("DF81000101"));
        TCOMPARE(toByteArray(1,   129), CFByteArray::fromHex("DF81010101"));
        TCOMPARE(toByteArray(1,   255), CFByteArray::fromHex("DF817F0101"));
        TCOMPARE(toByteArray(1,   256), CFByteArray::fromHex("DF82000101"));
        TCOMPARE(toByteArray(1, 16383), CFByteArray::fromHex("DFFF7F0101"));
        TCOMPARE(toByteArray(1, 16384), CFByteArray::fromHex("DF8180000101"));

        TCOMPARE(toByteArray(CFByteArray(),   3), CFByteArray::fromHex(""));
        TCOMPARE(toByteArray(CFByteArray(""), 3), CFByteArray::fromHex("C300"));
    }

    void test_fromByteArray()
    {
        TCOMPARE(fromByteArray<int>(CFByteArray::fromHex("")), 0);
        TCOMPARE(fromByteArray<int>(CFByteArray::fromHex("C000")), 0);
        TCOMPARE(fromByteArray<int>(CFByteArray::fromHex("C08100")), 0);
        TCOMPARE(fromByteArray<int>(CFByteArray::fromHex("C10101")), 1);
        TCOMPARE(fromByteArray<int>(CFByteArray::fromHex("C101FF")), -1);
        TCOMPARE(fromByteArray<String>(CFByteArray::fromHex("")), String());
        TCOMPARE(fromByteArray<String>(CFByteArray::fromHex("C000")), String());
        TVERIFY(!fromByteArray<String>(CFByteArray::fromHex("C000")).isNull());
        TCOMPARE(fromByteArray<String>(CFByteArray::fromHex("C08100")), String());
        TVERIFY(fromByteArray<String>(CFByteArray::fromHex("C08100")).isNull());
        TCOMPARE(fromByteArray<String>(CFByteArray::fromHex("C103626C61")), String("bla"));
    }

    void test_sizes()
    {
        TCOMPARE((int)sizeof(float      ),  4);
        TCOMPARE((int)sizeof(double     ),  8);
        TCOMPARE((int)sizeof(long double), 16);
    }

    void test_readAndCall()
    {
        BERSerializer ser;
        ser << 34 << "bla";
        BERDeserializer deser(ser.data());
        int i = 0;
        String s;
        readAndCall<int, const String &>(deser, [&](int pi, const String & ps) { i = pi; s = ps; });
        TCOMPARE(i, 34);
        TCOMPARE(s, String("bla"));
    }
};

ADD_TEST(Util_Test)

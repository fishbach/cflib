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

class Util_Test : public cflib::util::TestBase
{
public:
    std::vector<cflib::util::TestMethod> testMethods() const override {
        auto self = const_cast<Util_Test *>(this);
        return {
            {"test_flatten", [self]() { self->test_flatten(); }},
            {"test_CFString", [self]() { self->test_CFString(); }},
            {"test_CFByteArray", [self]() { self->test_CFByteArray(); }},
            {"test_deflate", [self]() { self->test_deflate(); }},
            {"test_tupleCompare", [self]() { self->test_tupleCompare(); }},
            {"test_callWithTupleParams", [self]() { self->test_callWithTupleParams(); }}
        };
    }

    void test_flatten()
    {
        QCOMPARE(flatten(CFString()), CFString());
        QCOMPARE(flatten(""), CFString(""));
        QCOMPARE(flatten("      \r\n"), CFString(""));
        QCOMPARE(flatten("     ab_c & 1.2-3\r\n"), CFString("ab_c_1.2-3"));
        QCOMPARE(flatten("     _ \r\n_"), CFString("_"));
    }

    void test_CFString()
    {
        QVERIFY(CFString().isNull());
        QVERIFY(!CFString("").isNull());
    }

    void test_CFByteArray()
    {
        CFByteArray ba;
        QVERIFY(ba.isNull());
        ba.reserve(997);
        QVERIFY(ba.capacity() >= (cfsize_t)997);
        ba.resize(123);
        QCOMPARE(ba.size(), (cfsize_t)123);
        QVERIFY(ba.capacity() >= (cfsize_t)997);
        ba.resize(0);
        QVERIFY(!ba.isNull());
        QCOMPARE(ba.size(), (cfsize_t)0);
        QVERIFY(ba.capacity() >= (cfsize_t)997);
        ba.clear();
        QVERIFY(ba.isNull());
        QVERIFY(!CFByteArray("").isNull());
    }

    void test_deflate()
    {
        auto checkDeflate = [](const CFByteArray & source, const CFByteArray & enc, int level) -> bool {
            CFByteArray data = source;
            deflateRaw(data, level);
            if (data != CFByteArray::fromHex(enc)) {
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

        QVERIFY(checkDeflate(CFByteArray(),                 "00",                     1));
        QVERIFY(checkDeflate(CFByteArray(),                 "00",                     0));
        QVERIFY(checkDeflate(CFByteArray("\0", 1),          "620000",                 1));
        QVERIFY(checkDeflate(CFByteArray("A"),              "720400",                 1));
        QVERIFY(checkDeflate(CFByteArray("A"),              "000100feff4100",         0));
        QVERIFY(checkDeflate(CFByteArray("bc"),             "4a4a0600",               1));
        QVERIFY(checkDeflate(CFByteArray("Hello"),          "f248cdc9c90700",         1));
        QVERIFY(checkDeflate(CFByteArray("Hello"),          "000500faff48656c6c6f00", 0));
        CFByteArray data;
        inflateRaw(data);
        QVERIFY(data.isEmpty());
        data = "\x00";
        inflateRaw(data);
        QVERIFY(data.isEmpty());
    }

    void test_tupleCompare()
    {
        QVERIFY( equal(std::tuple<int, float>(2, 3.14f), 2, 3.14f));
        QVERIFY(!equal(std::tuple<int, float>(2, 3.14f), 2, 3.2f));
        QVERIFY( equal(std::tuple<int, float>(2, 3.14f), 2));
        QVERIFY(!equal(std::tuple<int, float>(2, 3.14f), 3));
        QVERIFY( equal(std::tuple<int, float>(2, 3.14f)));

        QVERIFY( partialEqual(std::tuple<int, float>(2, 3.14f), 2, 2, 3.14f));
        QVERIFY(!partialEqual(std::tuple<int, float>(2, 3.14f), 2, 2, 3.2f));
        QVERIFY(!partialEqual(std::tuple<int, float>(2, 3.14f), 2, 3, 3.14f));
        QVERIFY( partialEqual(std::tuple<int, float>(2, 3.14f), 2, 2));
        QVERIFY(!partialEqual(std::tuple<int, float>(2, 3.14f), 2, 3));
        QVERIFY( partialEqual(std::tuple<int, float>(2, 3.14f), 2));

        QVERIFY( partialEqual(std::tuple<int, float>(2, 3.14f), 1, 2, 3.14f));
        QVERIFY( partialEqual(std::tuple<int, float>(2, 3.14f), 1, 2, 3.2f));
        QVERIFY(!partialEqual(std::tuple<int, float>(2, 3.14f), 1, 3, 3.14f));
        QVERIFY( partialEqual(std::tuple<int, float>(2, 3.14f), 1, 2));
        QVERIFY(!partialEqual(std::tuple<int, float>(2, 3.14f), 1, 3));
        QVERIFY( partialEqual(std::tuple<int, float>(2, 3.14f), 1));

        QVERIFY( partialEqual(std::tuple<int, float>(2, 3.14f), 0, 3, 3.2f));
        QVERIFY( partialEqual(std::tuple<int, float>(2, 3.14f), 0));
    }

    void test_callWithTupleParams()
    {
        int i = 0;
        float f = 0.0;
        callWithTupleParams<void>([&](int pi, float pf) { i = pi; f = pf; }, std::tuple<int, float>(2, 3.14f));
        QCOMPARE(i, 2);
        QCOMPARE(f, 3.14f);
        callWithTupleParams<void>([&](int & i, float pf) { ++i; f = pf; }, std::tuple<float>(2.34f), i);
        QCOMPARE(i, 3);
        QCOMPARE(f, 2.34f);
    }

};
ADD_TEST(Util_Test)

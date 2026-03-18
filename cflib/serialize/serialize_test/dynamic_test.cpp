/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/serialize/serialize_test/dynamic.h>
#include <cflib/util/test.h>

using namespace cflib::serialize;

class Dynamic_Test : public cflib::util::TestBase
{
public:
    std::vector<cflib::util::TestMethod> testMethods() const override {
        auto self = const_cast<Dynamic_Test *>(this);
        return {
            {"serializeNull", [self]() { self->serializeNull(); }},
            {"serialize",     [self]() { self->serialize(); }}
        };
    }

    void serializeNull()
    {
        DynamicUse in;
        in.y = 23;
        BERSerializer ser;
        ser << in;

        DynamicUse out;
        BERDeserializer deser(ser.data());
        deser >> out;

        QVERIFY(in.y == out.y);
        QVERIFY(!in.d);
        QVERIFY(in.e.empty());
        QVERIFY(in.z == out.z);
    }

    void serialize()
    {
        DynamicUse in;
        in.y = 23;
        DynamicA * dynA = new DynamicA();
        dynA->a = 45;
        in.d.reset(dynA);
        DynamicB * dynB = new DynamicB();
        dynB->b = 123.45;
        in.e.push_back(CFSharedPtr<DynamicBase>(dynB));
        BERSerializer ser;
        ser << in;

        DynamicUse out;
        BERDeserializer deser(ser.data());
        deser >> out;

        QCOMPARE(out.y, in.y);
        QVERIFY(!!in.d);
        QCOMPARE(std::dynamic_pointer_cast<DynamicA>(in.d)->a, 45);
        QCOMPARE((int)in.e.size(), 1);
        QCOMPARE(std::dynamic_pointer_cast<DynamicB>(in.e[0])->b, 123.45);
        QCOMPARE(out.z, in.z);
    }
};

ADD_TEST(Dynamic_Test)

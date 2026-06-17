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

        TVERIFY(in.y == out.y);
        TVERIFY(!in.d);
        TVERIFY(in.e.isEmpty());
        TVERIFY(in.z == out.z);
    }

    void serialize()
    {
        DynamicUse in;
        in.y = 23;
        DynamicA * dynA = new DynamicA();
        dynA->x = 42;
        dynA->t1 = 1;
        dynA->t2 = 2;
        dynA->t3 = 3;
        dynA->a = 45;
        in.d.reset(dynA);
        DynamicB * dynB = new DynamicB();
        dynB->b = 123.45;
        in.e.push_back(SharedPtr<DynamicBase>(dynB));
        in.z = 666;
        BERSerializer ser;
        ser << in;

        DynamicUse out;
        BERDeserializer deser(ser.data());
        deser >> out;

        TCOMPARE(out.y, 23);
        TVERIFY(out.d);
        auto da = cflib::base::dynamic_pointer_cast<DynamicA>(out.d);
        TVERIFY(da);
        TCOMPARE(da->x, 42);
        TCOMPARE(da->t1, 1);
        TCOMPARE(da->t2, 2);
        TCOMPARE(da->t3, 3);
        TCOMPARE(da->a, 45);
        TCOMPARE((int)out.e.size(), 1);
        auto db = cflib::base::dynamic_pointer_cast<DynamicB>(out.e[0]);
        TVERIFY(db);
        TCOMPARE(db->b, 123.45);
        TCOMPARE(out.z, 666);
    }
};

ADD_TEST(Dynamic_Test)

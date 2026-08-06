/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/serialize/serialize_test/dynamic.h>
#include <cflib/util/test.h>

using namespace cflib::serialize;

TEST_SUITE("Dynamic") {

TEST_CASE("Dynamic: serializeNull")
{
    DynamicUse in;
    in.y = 23;
    BERSerializer ser;
    ser << in;

    DynamicUse out;
    BERDeserializer deser(ser.data());
    deser >> out;

    REQUIRE(in.y == out.y);
    REQUIRE(!in.d);
    REQUIRE(in.e.isEmpty());
    REQUIRE(in.z == out.z);
}

TEST_CASE("Dynamic: serialize")
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

    REQUIRE_EQ(out.y, 23);
    REQUIRE(out.d);
    auto da = cflib::base::dynamic_pointer_cast<DynamicA>(out.d);
    REQUIRE(da);
    REQUIRE_EQ(da->x, 42);
    REQUIRE_EQ(da->t1, 1);
    REQUIRE_EQ(da->t2, 2);
    REQUIRE_EQ(da->t3, 3);
    REQUIRE_EQ(da->a, 45);
    REQUIRE_EQ((int)out.e.size(), 1);
    auto db = cflib::base::dynamic_pointer_cast<DynamicB>(out.e[0]);
    REQUIRE(db);
    REQUIRE_EQ(db->b, 123.45);
    REQUIRE_EQ(out.z, 666);
}

}

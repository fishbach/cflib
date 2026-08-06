/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/net/net_test/gentest.h>
#include <cflib/net/net_test/test/gentest2.h>
#include <cflib/util/test.h>

using namespace cflib::serialize;

TEST_SUITE("GenBER") {

TEST_CASE("GenBER: serialize")
{
    GenTest1 gt1;
    gt1.a = 3;
    gt1.c = 4;
    gt1.d = "xy";
    gentest::GenTest3 gt3;
    gt3.a = 5;
    gt3.c = 6;
    gt3.d = "ab";
    gt3.e = 7;
    gt3.f = 8;
    BERSerializer ser;
    ser << gt1 << gt3;
    REQUIRE_EQ(ser.data(), ByteArray::fromHex(
        "                    E10A C20103 C40104 C5027879"
        "E219 C10500E4440B6F E20A C20105 C40106 C5026162 C30107 C40108"
    ));
}

TEST_CASE("GenBER: deserialize")
{
    BERDeserializer ser(ByteArray::fromHex(
        "                    E10A C20103 C40104 C5027879"
        "E219 C10500E4440B6F E20A C20105 C40106 C5026162 C30107 C40108"
    ));
    GenTest1 gt1;
    gentest::GenTest3 gt3;
    ser >> gt1 >> gt3;
    REQUIRE_EQ(gt1.a, 3);
    REQUIRE_EQ(gt1.c, 4);
    REQUIRE_EQ(gt1.d, String("xy"));
    REQUIRE_EQ(gt3.a, 5);
    REQUIRE_EQ(gt3.c, 6);
    REQUIRE_EQ(gt3.d, String("ab"));
    REQUIRE_EQ(gt3.e, 7);
    REQUIRE_EQ(gt3.f, 8);
}

TEST_CASE("GenBER: template_ser")
{
    gentest::gentest2::GenTest4 gt4;
    gt4.push_back(String("AB"));
    gt4.push_back(String(""));
    gt4.a = 7;
    gt4.b << 13 << 17;
    BERSerializer ser;
    ser << gt4;
    REQUIRE_EQ(ser.data(), ByteArray::fromHex(
        "E11A"
        "C10500FEDBD07E"
        "E206 C0024142 C000"
        "C30107"
        "E406 C0010D C00111"
    ));
}

TEST_CASE("GenBER: template_deser")
{
    BERDeserializer ser(ByteArray::fromHex(
        "E11A"
        "C10500FEDBD07E"
        "E206 C0024142 C000"
        "C30107"
        "E406 C0010D C00111"
    ));
    gentest::gentest2::GenTest4 gt4;
    ser >> gt4;
    REQUIRE_EQ((int)gt4.size(), 2);
    REQUIRE_EQ(gt4[0], String("AB"));
    REQUIRE(!gt4[1].isNull());
    REQUIRE(gt4[1].isEmpty());
    REQUIRE_EQ(gt4.a, 7);
    REQUIRE_EQ((int)gt4.b.size(), 2);
    REQUIRE_EQ(gt4.b[0], 13);
    REQUIRE_EQ(gt4.b[1], 17);
}

}

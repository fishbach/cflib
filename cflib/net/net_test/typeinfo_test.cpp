/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/net/net_test/gentest.h>
#include <cflib/net/net_test/test/gentest2.h>
#include <cflib/util/test.h>

#include <cflib/serialize/impl/serializetypeinfoimpl.h>

using namespace cflib::serialize;

TEST_SUITE("TypeInfo") {

TEST_CASE("TypeInfo: toString")
{
    REQUIRE_EQ(GenTestRMI::serializeTypeInfo().toString(), String(
        "GenTestRMI{void f3(int32, String), List<int32> f4(), int32 f5(int32 x, int32 y), void f6()}"));
    REQUIRE_EQ(GenTest1::serializeTypeInfo().toString(), String(
        "GenTest1{int32 a, , int32 c, String d}"));
    REQUIRE_EQ(GenTest2::serializeTypeInfo().toString(), String(
        "GenTest2{GenTest1{int32 a, , int32 c, String d} a, int32 b}"));
    REQUIRE_EQ(gentest::GenTest3::serializeTypeInfo().toString(), String(
        "gentest::GenTest3[GenTest1{int32 a, , int32 c, String d}]{int32 e, int32 f}"));
    REQUIRE_EQ(gentest::GenTest3::Inner1::serializeTypeInfo().toString(), String(
        "gentest::GenTest3::Inner1{int32 a}"));
    REQUIRE_EQ(gentest::GenTest3::Inner2::serializeTypeInfo().toString(), String(
        "gentest::GenTest3::Inner2{}"));
    REQUIRE_EQ(gentest::gentest2::GenTest4::serializeTypeInfo().toString(), String(
        "gentest::gentest2::GenTest4[List<String>]"
        "{int32 a, List<int32> b, List<GenTest2{GenTest1{int32 a, , int32 c, String d} a, int32 b}> c}"));
    REQUIRE_EQ(GenTest6::serializeTypeInfo().toString(), String(
        "GenTest6{int32 a}"));
}

}

/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "permission_test.h"

#include <cflib/util/test.h>

using namespace cflib::serialize;

TEST_SUITE("Permission") {

TEST_CASE("Permission: basic")
{
    REQUIRE(TestPerm1.name                            == "TestPerm1");
    REQUIRE(TestPerm1.description                     == "some comment.");
    REQUIRE(TestPerm1.id                              == 0);
    REQUIRE(Permission::lookup("TestPerm1")           == &TestPerm1);
    REQUIRE(test::perm::TestPerm2.name                == "test.perm.TestPerm2");
    REQUIRE(test::perm::TestPerm2.description         == "another, description?!");
    REQUIRE(test::perm::TestPerm2.id                  == 0);
    REQUIRE(Permission::lookup("test.perm.TestPerm2") == &test::perm::TestPerm2);
    REQUIRE(test::perm::TestPerm3.name                == "test.perm.TestPerm3");
    REQUIRE(test::perm::TestPerm3.id                  == 0);
    REQUIRE(Permission::lookup("test.perm.TestPerm3") == &test::perm::TestPerm3);
    REQUIRE(testPerm::TestPerm4.name                  == "testPerm.TestPerm4");
    REQUIRE(testPerm::TestPerm4.id                    == 0);
    REQUIRE(Permission::lookup("testPerm.TestPerm4")  == &testPerm::TestPerm4);
}

}

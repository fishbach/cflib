/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "permission_test.h"

#include <cflib/util/test.h>

using namespace cflib::serialize;

class Permission_Test : public cflib::util::TestBase
{
public:
    std::vector<cflib::util::TestMethod> testMethods() const override {
        auto self = const_cast<Permission_Test *>(this);
        return {
            {"basic", [self]() { self->basic(); }}
        };
    }

    void basic()
    {
        TVERIFY(TestPerm1.name                              == "TestPerm1");
        TVERIFY(TestPerm1.description                       == "some comment.");
        TVERIFY(TestPerm1.id                                == 0);
        TVERIFY(CFPermission::lookup("TestPerm1")           == &TestPerm1);
        TVERIFY(test::perm::TestPerm2.name                  == "test.perm.TestPerm2");
        TVERIFY(test::perm::TestPerm2.description           == "another, description?!");
        TVERIFY(test::perm::TestPerm2.id                    == 0);
        TVERIFY(CFPermission::lookup("test.perm.TestPerm2") == &test::perm::TestPerm2);
        TVERIFY(test::perm::TestPerm3.name                  == "test.perm.TestPerm3");
        TVERIFY(test::perm::TestPerm3.id                    == 0);
        TVERIFY(CFPermission::lookup("test.perm.TestPerm3") == &test::perm::TestPerm3);
        TVERIFY(testPerm::TestPerm4.name                    == "testPerm.TestPerm4");
        TVERIFY(testPerm::TestPerm4.id                      == 0);
        TVERIFY(CFPermission::lookup("testPerm.TestPerm4")  == &testPerm::TestPerm4);
    }
};

ADD_TEST(Permission_Test)

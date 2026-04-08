/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/util/test.h>

int sqlite_test();

class SQLite_test : public cflib::util::TestBase
{
public:
    std::vector<cflib::util::TestMethod> testMethods() const override {
        auto self = const_cast<SQLite_test *>(this);
        return {
            {"test", [self]() { self->test(); }}
        };
    }

private:
    void test()
    {
        TCOMPARE(sqlite_test(), 0);
    }
};
ADD_TEST(SQLite_test)

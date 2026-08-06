/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/util/test.h>
#include <cflib/util/sig.h>

using namespace cflib::util;

TEST_SUITE("Sig") {

TEST_CASE("Sig: simple")
{
    int callCount = 0;
    Sig<void (int)> sig;
    sig.bind([&callCount](int x) { callCount += x; });
    REQUIRE_EQ(callCount, 0);
    sig(3);
    REQUIRE_EQ(callCount, 3);
    sig(5);
    REQUIRE_EQ(callCount, 8);
    sig.bind([&callCount](int x) { callCount += 2 * x; });
    sig(2);
    REQUIRE_EQ(callCount, 14);
    sig.unbindAll();
    sig(7);
    REQUIRE_EQ(callCount, 14);
}

TEST_CASE("Sig: return")
{
    Sig<int (int)> sig;
    sig.bind([](int x) { return x + 7; });
    REQUIRE_EQ(sig(3), 10);
    REQUIRE_EQ(sig(7), 14);
}

TEST_CASE("Sig: ref")
{
    Sig<void (int &)> sig;
    sig.bind([](int & x) { x += 2; });
    sig.bind([](int & x) { x *= 3; });
    int x = 5;
    sig(x);
    REQUIRE_EQ(x, 21);
}

TEST_CASE("Sig: member")
{
    class Test
    {
    public:
        void test() { ++x; }
        int rv() const { return x + 7; }
        int x;
    };
    Test t;
    t.x = 0;

    Sig<void ()> sig;
    sig.bind(&t, &Test::test);
    REQUIRE_EQ(t.x, 0);
    sig();
    REQUIRE_EQ(t.x, 1);

    Sig<int ()> sig2;
    sig2.bind(&t, &Test::rv);
    REQUIRE_EQ(sig2(), 8);
    sig();

    const Test & t2(t);
    Sig<int ()> sig3;
    sig3.bind(&t2, &Test::rv);
    REQUIRE_EQ(sig3(), 9);
}

}

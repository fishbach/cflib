/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/util/test.h>
#include <cflib/util/sig.h>

using namespace cflib::util;

class Sig_Test : public cflib::util::TestBase
{
public:
    std::vector<cflib::util::TestMethod> testMethods() const override {
        auto self = const_cast<Sig_Test *>(this);
        return {
            {"simple_test", [self]() { self->simple_test(); }},
            {"return_test", [self]() { self->return_test(); }},
            {"ref_test", [self]() { self->ref_test(); }},
            {"member_test", [self]() { self->member_test(); }}
        };
    }

    void simple_test()
    {
        int callCount = 0;
        Sig<void (int)> sig;
        sig.bind([&callCount](int x) { callCount += x; });
        QCOMPARE(callCount, 0);
        sig(3);
        QCOMPARE(callCount, 3);
        sig(5);
        QCOMPARE(callCount, 8);
        sig.bind([&callCount](int x) { callCount += 2 * x; });
        sig(2);
        QCOMPARE(callCount, 14);
        sig.unbindAll();
        sig(7);
        QCOMPARE(callCount, 14);
    }

    void return_test()
    {
        Sig<int (int)> sig;
        sig.bind([](int x) { return x + 7; });
        QCOMPARE(sig(3), 10);
        QCOMPARE(sig(7), 14);
    }

    void ref_test()
    {
        Sig<void (int &)> sig;
        sig.bind([](int & x) { x += 2; });
        sig.bind([](int & x) { x *= 3; });
        int x = 5;
        sig(x);
        QCOMPARE(x, 21);
    }

    void member_test()
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
        QCOMPARE(t.x, 0);
        sig();
        QCOMPARE(t.x, 1);

        Sig<int ()> sig2;
        sig2.bind(&t, &Test::rv);
        QCOMPARE(sig2(), 8);
        sig();

        const Test & t2(t);
        Sig<int ()> sig3;
        sig3.bind(&t2, &Test::rv);
        QCOMPARE(sig3(), 9);
    }

};
ADD_TEST(Sig_Test)

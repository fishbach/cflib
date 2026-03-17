/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/crypt/util.h>
#include <cflib/util/test.h>

using namespace cflib::crypt;

class Util_test : public cflib::util::TestBase
{
public:
    std::vector<cflib::util::TestMethod> testMethods() const override {
        auto self = const_cast<Util_test *>(this);
        return {
            {"test_random",          [self]() { self->test_random(); }},
            {"test_randomId",        [self]() { self->test_randomId(); }},
            {"test_randomUInt32",    [self]() { self->test_randomUInt32(); }},
            {"test_randomUInt64",    [self]() { self->test_randomUInt64(); }},
            {"test_memorableRandom", [self]() { self->test_memorableRandom(); }},
            {"test_hashPassword",    [self]() { self->test_hashPassword(); }},
            {"test_sha1",            [self]() { self->test_sha1(); }},
            {"test_sha1ForWebSocket",[self]() { self->test_sha1ForWebSocket(); }}
        };
    }

    void test_random()
    {
        QCOMPARE((int)random( 0).size(),  0);
        QCOMPARE((int)random( 1).size(),  1);
        QCOMPARE((int)random(13).size(), 13);
        QVERIFY(random(8) != random(8));
    }

    void test_randomId()
    {
        QCOMPARE((int)randomId().size(), 40);
        QVERIFY(randomId() != randomId());
    }

    void test_randomUInt32()
    {
        QVERIFY(randomUInt32() != randomUInt32());
    }

    void test_randomUInt64()
    {
        QVERIFY(randomUInt64() != randomUInt64());
    }

    void test_memorableRandom()
    {
        fprintf(stdout, "random: '%s'\n", memorableRandom().data());
        QCOMPARE((int)memorableRandom().size(), 8);
        QVERIFY(memorableRandom() != memorableRandom());
    }

    void test_hashPassword()
    {
        QVERIFY(hashPassword("pwd") != hashPassword("pwd"));
        QVERIFY(checkPassword("", hashPassword("")));
        QVERIFY(checkPassword("p", hashPassword("p")));
        QVERIFY(checkPassword("abcABC123!@#,.", hashPassword("abcABC123!@#,.")));
        QVERIFY(!checkPassword("pwd1", hashPassword("pwd2")));
    }

    void test_sha1()
    {
        QCOMPARE(sha1(""),    CFByteArray::fromHex("da39a3ee5e6b4b0d3255bfef95601890afd80709"));
        QCOMPARE(sha1("a"),   CFByteArray::fromHex("86f7e437faa5a7fce15d1ddcb9eaeaea377667b8"));
        QCOMPARE(sha1("abc"), CFByteArray::fromHex("a9993e364706816aba3e25717850c26c9cd0d89d"));
    }

    void test_sha1ForWebSocket()
    {
        QCOMPARE(
            sha1("x3JJHMbDL1EzLkh9GBhXDw==258EAFA5-E914-47DA-95CA-C5AB0DC85B11").toBase64(),
            CFByteArray("HSmrc0sMlYUkAGmm5OPpG2HaGWk=")
        );
    }
};

ADD_TEST(Util_test)

/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/crypt/util.h>
#include <cflib/util/test.h>

#include <format>
#include <iostream>

using namespace cflib::crypt;

TEST_SUITE("crypt") {

TEST_CASE("crypt: random")
{
    REQUIRE_EQ((int)random( 0).size(),  0);
    REQUIRE_EQ((int)random( 1).size(),  1);
    REQUIRE_EQ((int)random(13).size(), 13);
    REQUIRE(random(8) != random(8));
}

TEST_CASE("crypt: randomId")
{
    REQUIRE_EQ((int)randomId().size(), 40);
    REQUIRE(randomId() != randomId());
}

TEST_CASE("crypt: randomUInt32")
{
    REQUIRE(randomUInt32() != randomUInt32());
}

TEST_CASE("crypt: randomUInt64")
{
    REQUIRE(randomUInt64() != randomUInt64());
}

TEST_CASE("crypt: memorableRandom")
{
    std::cout << std::format("random: '{}'\n", memorableRandom().toStdStringView());
    REQUIRE_EQ((int)memorableRandom().size(), 8);
    REQUIRE(memorableRandom() != memorableRandom());
}

TEST_CASE("crypt: hashPassword")
{
    REQUIRE(hashPassword("pwd") != hashPassword("pwd"));
    REQUIRE(checkPassword("", hashPassword("")));
    REQUIRE(checkPassword("p", hashPassword("p")));
    REQUIRE(checkPassword("abcABC123!@#,.", hashPassword("abcABC123!@#,.")));
    REQUIRE(!checkPassword("pwd1", hashPassword("pwd2")));
}

TEST_CASE("crypt: sha1")
{
    REQUIRE_EQ(sha1(""),    ByteArray::fromHex("da39a3ee5e6b4b0d3255bfef95601890afd80709"));
    REQUIRE_EQ(sha1("a"),   ByteArray::fromHex("86f7e437faa5a7fce15d1ddcb9eaeaea377667b8"));
    REQUIRE_EQ(sha1("abc"), ByteArray::fromHex("a9993e364706816aba3e25717850c26c9cd0d89d"));
}

TEST_CASE("crypt: sha1ForWebSocket")
{
    REQUIRE_EQ(
        sha1("x3JJHMbDL1EzLkh9GBhXDw==258EAFA5-E914-47DA-95CA-C5AB0DC85B11").toBase64(),
        ByteArray("HSmrc0sMlYUkAGmm5OPpG2HaGWk=")
    );
}

}

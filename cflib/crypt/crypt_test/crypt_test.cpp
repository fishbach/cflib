/* Copyright (C) 2013-2014 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * cflib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * cflib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cflib. If not, see <http://www.gnu.org/licenses/>.
 */

#include <cflib/crypt/util.h>
#include <cflib/util/test.h>

using namespace cflib::crypt;

class Crypt_Test: public QObject
{
	Q_OBJECT
private slots:

	void initTestCase()
	{
		QVERIFY(initCrypto());
	}

	void test_random()
	{
		QCOMPARE(random( 0).size(),  0);
		QCOMPARE(random( 1).size(),  1);
		QCOMPARE(random(13).size(), 13);
		QVERIFY(random(8) != random(8));
	}

	void test_randomId()
	{
		QCOMPARE(randomId().size(), 40);
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
		QCOMPARE(memorableRandom().length(), 8);
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
		QCOMPARE(sha1(""),    QByteArray::fromHex("da39a3ee5e6b4b0d3255bfef95601890afd80709"));
		QCOMPARE(sha1("a"),   QByteArray::fromHex("86f7e437faa5a7fce15d1ddcb9eaeaea377667b8"));
		QCOMPARE(sha1("abc"), QByteArray::fromHex("a9993e364706816aba3e25717850c26c9cd0d89d"));
	}

	void test_sha1ForWebSocket()
	{
		QCOMPARE(
			sha1("x3JJHMbDL1EzLkh9GBhXDw==258EAFA5-E914-47DA-95CA-C5AB0DC85B11").toBase64(),
			QByteArray("HSmrc0sMlYUkAGmm5OPpG2HaGWk=")
		);
	}

};
#include "crypt_test.moc"
ADD_TEST(Crypt_Test)

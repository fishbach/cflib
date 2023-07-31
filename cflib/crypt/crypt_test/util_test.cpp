/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/crypt/util.h>
#include <cflib/util/test.h>

using namespace cflib::crypt;

class Util_test : public QObject
{
	Q_OBJECT
private slots:

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
		QTextStream(stdout) << "random: '" << memorableRandom() << "'" << Qt::endl;
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
#include "util_test.moc"
ADD_TEST(Util_test)

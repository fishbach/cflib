/* Copyright (C) 2013-2016 Christian Fischbach <cf@cflib.de>
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

#include <cflib/serialize/serializeber.h>
#include <cflib/serialize/serialize_test/gentest.h>
#include <cflib/serialize/serialize_test/test/gentest2.h>
#include <cflib/util/test.h>

using namespace cflib::serialize;

class Gen_BER_Test: public QObject
{
	Q_OBJECT
private slots:

	void serialize()
	{
		GenTest1 gt1;
		gt1.a = 3;
		gt1.c = 4;
		gt1.d = "xy";
		gentest::GenTest3 gt3;
		gt3.a = 5;
		gt3.c = 6;
		gt3.d = "ab";
		gt3.e = 7;
		gt3.f = 8;
		BERSerializer ser;
		ser << gt1 << gt3;
		QCOMPARE(ser.data(), QByteArray::fromHex(
			"     E10A C10103 C30104 C4027879"
			"E212 E10A C10105 C30106 C4026162 C20107 C30108"
		));
	}

	void deserialize()
	{
		BERDeserializer ser(QByteArray::fromHex(
			"     E10A C10103 C30104 C4027879"
			"E212 E10A C10105 C30106 C4026162 C20107 C30108"
		));
		GenTest1 gt1;
		gentest::GenTest3 gt3;
		ser >> gt1 >> gt3;
		QCOMPARE(gt1.a, 3);
		QCOMPARE(gt1.c, 4);
		QCOMPARE(gt1.d, QString("xy"));
		QCOMPARE(gt3.a, 5);
		QCOMPARE(gt3.c, 6);
		QCOMPARE(gt3.d, QString("ab"));
		QCOMPARE(gt3.e, 7);
		QCOMPARE(gt3.f, 8);
	}

	void template_ser()
	{
		gentest::gentest2::GenTest4 gt4;
		gt4 << "AB";
		gt4 << "";
		gt4.a = 7;
		gt4.b << 13 << 17;
		BERSerializer ser;
		ser << gt4;
		QCOMPARE(ser.data(), QByteArray::fromHex(
			"E113"
			"E106 C0024142 C000"
			"C20107"
			"E306 C0010D C00111"
		));
	}

	void template_deser()
	{
		BERDeserializer ser(QByteArray::fromHex(
			"E113"
			"E106 C0024142 C000"
			"C20107"
			"E306 C0010D C00111"
		));
		gentest::gentest2::GenTest4 gt4;
		ser >> gt4;
		QCOMPARE(gt4.size(), 2);
		QCOMPARE(gt4[0], QString("AB"));
		QVERIFY(!gt4[1].isNull());
		QVERIFY(gt4[1].isEmpty());
		QCOMPARE(gt4.a, 7);
		QCOMPARE(gt4.b.size(), 2);
		QCOMPARE(gt4.b[0], 13);
		QCOMPARE(gt4.b[1], 17);
	}

};
#include "gen_ber_test.moc"
ADD_TEST(Gen_BER_Test)

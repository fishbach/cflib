/* Copyright (C) 2013-2017 Christian Fischbach <cf@cflib.de>
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

#include <cflib/serialize/serialize_test/dynamic.h>
#include <cflib/util/test.h>

using namespace cflib::serialize;

class Dynamic_Test: public QObject
{
	Q_OBJECT
private slots:

	void serializeNull()
	{
		DynamicUse in;
		in.y = 23;
		BERSerializer ser;
		ser << in;

		DynamicUse out;
		BERDeserializer deser(ser.data());
		deser >> out;

		QVERIFY(in.y == out.y);
		QVERIFY(in.d.isNull());
		QVERIFY(in.e.isEmpty());
		QVERIFY(in.z == out.z);
	}

	void serialize()
	{
		DynamicUse in;
		in.y = 23;
		DynamicA * dynA = new DynamicA();
		dynA->a = 45;
		in.d.reset(dynA);
		DynamicB * dynB = new DynamicB();
		dynB->b = 123.45;
		in.e << QSharedPointer<DynamicBase>(dynB);
		BERSerializer ser;
		ser << in;

		DynamicUse out;
		BERDeserializer deser(ser.data());
		deser >> out;

		QCOMPARE(out.y, in.y);
		QVERIFY(!in.d.isNull());
		QCOMPARE(in.d.dynamicCast<DynamicA>()->a, 45);
		QCOMPARE(in.e.size(), 1);
		QCOMPARE(in.e.first().dynamicCast<DynamicB>()->b, 123.45);
		QCOMPARE(out.z, in.z);
	}

};
#include "dynamic_test.moc"
ADD_TEST(Dynamic_Test)

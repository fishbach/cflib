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

#include <cflib/serialize/util.h>
#include <cflib/util/test.h>

using namespace cflib::serialize;

class Util_Test: public QObject
{
	Q_OBJECT
private slots:

	void test_getTLVLength()
	{
		QCOMPARE(getTLVLength(QByteArray::fromHex(""            )), -1);
		QCOMPARE(getTLVLength(QByteArray::fromHex("c1"          )), -1);
		QCOMPARE(getTLVLength(QByteArray::fromHex("c100"        )),  2);
		QCOMPARE(getTLVLength(QByteArray::fromHex("c18100"      )),  3);
		QCOMPARE(getTLVLength(QByteArray::fromHex("c180"        )), -2);
		QCOMPARE(getTLVLength(QByteArray::fromHex("c101"        )), -1);
		QCOMPARE(getTLVLength(QByteArray::fromHex("c10100"      )),  3);
		QCOMPARE(getTLVLength(QByteArray::fromHex("c1810100"    )),  4);
		QCOMPARE(getTLVLength(QByteArray::fromHex("c1847FFFFFF9")), -1);
		QCOMPARE(getTLVLength(QByteArray::fromHex("c1847FFFFFFA")), -3);
		QCOMPARE(getTLVLength(QByteArray::fromHex("c18480000000")), -3);
		QCOMPARE(getTLVLength(QByteArray::fromHex("c188"        )), -1);
		QCOMPARE(getTLVLength(QByteArray::fromHex("c189"        )), -3);
	}

	void test_toByteArray()
	{
		QCOMPARE(toByteArray(0), QByteArray::fromHex(""));
		QCOMPARE(toByteArray(1), QByteArray::fromHex("C10101"));
		QCOMPARE(toByteArray(-1), QByteArray::fromHex("C101FF"));
		QCOMPARE(toByteArray("bla"), QByteArray::fromHex("C103626C61"));
	}

	void test_fromByteArray()
	{
		QCOMPARE(fromByteArray<int>(QByteArray::fromHex("")), 0);
		QCOMPARE(fromByteArray<int>(QByteArray::fromHex("C10101")), 1);
		QCOMPARE(fromByteArray<int>(QByteArray::fromHex("C101FF")), -1);
		QCOMPARE(fromByteArray<QString>(QByteArray::fromHex("C103626C61")), QString("bla"));
	}

};
#include "util_test.moc"
ADD_TEST(Util_Test)

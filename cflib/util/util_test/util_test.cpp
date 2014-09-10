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

#include <cflib/util/test.h>
#include <cflib/util/util.h>

using namespace cflib::util;

class Util_Test: public QObject
{
	Q_OBJECT
private slots:

	void test_flatten()
	{
		QCOMPARE(flatten(QString()), QString());
		QCOMPARE(flatten(""), QString(""));
		QCOMPARE(flatten("\t  \r\n"), QString(""));
		QCOMPARE(flatten("\t ab_c & 1.2-3 öüäß\r\n"), QString("ab_c_1.2-3"));
		QCOMPARE(flatten("\t _ \r\n_"), QString("_"));
	}

};
#include "util_test.moc"
ADD_TEST(Util_Test)

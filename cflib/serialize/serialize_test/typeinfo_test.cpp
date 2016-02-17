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

#include <cflib/serialize/serialize_test/gentest.h>
#include <cflib/serialize/serialize_test/test/gentest2.h>
#include <cflib/util/test.h>

#include <cflib/serialize/impl/serializetypeinfoimpl.h>

using namespace cflib::serialize;

class TypeInfo_Test: public QObject
{
	Q_OBJECT
private slots:

	void toString()
	{
		QCOMPARE(GenTest1::serializeTypeInfo().toString(), QString(
			"GenTest1{int32 a,,int32 c,String d,"
			"void f3(int32,String),List<int32> f4(),int32 f5(int32 x,int32 y),void f6()}"));
		QCOMPARE(GenTest2::serializeTypeInfo().toString(), QString(
			"GenTest2{GenTest1{int32 a,,int32 c,String d,"
			"void f3(int32,String),List<int32> f4(),int32 f5(int32 x,int32 y),void f6()} a,int32 b}"));
		QCOMPARE(gentest::GenTest3::serializeTypeInfo().toString(), QString(
			"gentest::GenTest3[GenTest1{int32 a,,int32 c,String d"
			",void f3(int32,String),List<int32> f4(),int32 f5(int32 x,int32 y),void f6()}]{int32 e,int32 f}"));
		QCOMPARE(gentest::GenTest3::Inner1::serializeTypeInfo().toString(), QString(
			"gentest::GenTest3::Inner1{int32 a}"));
		QCOMPARE(gentest::GenTest3::Inner2::serializeTypeInfo().toString(), QString(
			"gentest::GenTest3::Inner2{}"));
		QCOMPARE(gentest::gentest2::GenTest4::serializeTypeInfo().toString(), QString(
			"gentest::gentest2::GenTest4[List<String>]"
			"{int32 a,List<int32> b,List<GenTest2{GenTest1{int32 a,,int32 c,String d,"
			"void f3(int32,String),List<int32> f4(),int32 f5(int32 x,int32 y),void f6()} a,int32 b}> c}"));
		QCOMPARE(GenTest6::serializeTypeInfo().toString(), QString(
			"GenTest6{int32 a}"));
	}

};
#include "typeinfo_test.moc"
ADD_TEST(TypeInfo_Test)

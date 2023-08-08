/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/net/net_test/gentest.h>
#include <cflib/net/net_test/test/gentest2.h>
#include <cflib/util/test.h>

#include <cflib/serialize/impl/serializetypeinfoimpl.h>

using namespace cflib::serialize;

class TypeInfo_Test: public QObject
{
	Q_OBJECT
private slots:

	void toString()
	{
		QCOMPARE(GenTestRMI::serializeTypeInfo().toString(), QString(
			"GenTestRMI{void f3(int32,String),List<int32> f4(),int32 f5(int32 x,int32 y),void f6()}"));
		QCOMPARE(GenTest1::serializeTypeInfo().toString(), QString(
			"GenTest1{int32 a,,int32 c,String d}"));
		QCOMPARE(GenTest2::serializeTypeInfo().toString(), QString(
			"GenTest2{GenTest1{int32 a,,int32 c,String d} a,int32 b}"));
		QCOMPARE(gentest::GenTest3::serializeTypeInfo().toString(), QString(
			"gentest::GenTest3[GenTest1{int32 a,,int32 c,String d}]{int32 e,int32 f}"));
		QCOMPARE(gentest::GenTest3::Inner1::serializeTypeInfo().toString(), QString(
			"gentest::GenTest3::Inner1{int32 a}"));
		QCOMPARE(gentest::GenTest3::Inner2::serializeTypeInfo().toString(), QString(
			"gentest::GenTest3::Inner2{}"));
		QCOMPARE(gentest::gentest2::GenTest4::serializeTypeInfo().toString(), QString(
			"gentest::gentest2::GenTest4[List<String>]"
			"{int32 a,List<int32> b,List<GenTest2{GenTest1{int32 a,,int32 c,String d} a,int32 b}> c}"));
		QCOMPARE(GenTest6::serializeTypeInfo().toString(), QString(
			"GenTest6{int32 a}"));
	}

};
#include "typeinfo_test.moc"
ADD_TEST(TypeInfo_Test)

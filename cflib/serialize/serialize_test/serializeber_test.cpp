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

#include <cflib/serialize/serialize_test/test.h>
#include <cflib/util/test.h>

using namespace cflib::serialize;

namespace {

QTextStream out(stdout);

template<typename T>
bool checkSer(T val, const char * hex)
{
	BERSerializer ser;
	ser << val;
	const QByteArray expected = QByteArray::fromHex(hex);
	if (ser.data() != expected) {
		out << "serialized hex differs:" << endl
			<< "is       : " << ser.data().toHex() << endl
			<< "expected : " << expected  .toHex() << endl;
		return false;
	}
	return true;
}

template<typename T>
bool checkDeser(T val, const char * hex)
{
	const QByteArray expected = QByteArray::fromHex(hex);
	BERDeserializer deser(expected);
	T test; deser >> test;
	if (test != val) {
		out << "deserialized values differ:" << endl
			<< "is       : " << test << endl
			<< "expected : " << val  << endl;
		return false;
	}
	return true;
}

template<typename T>
bool checkDeserNull(T val, const char * hex)
{
	const QByteArray expected = QByteArray::fromHex(hex);
	BERDeserializer deser(expected);
	T test; deser >> test;
	if (test.isNull() != val.isNull()) {
		out << "deserialized isNull differs:" << endl
			<< "is       : " << test.isNull() << endl
			<< "expected : " << val.isNull()  << endl;
		return false;
	}
	return true;
}

template<typename T>
bool checkSerDeser(T val, const char * hex)
{
	return checkSer(val, hex) && checkDeser(val, hex);
}

template<typename T>
bool checkSerDeserNull(T val, const char * hex)
{
	return checkSer(val, hex) && checkDeser(val, hex) && checkDeserNull(val, hex);
}

bool testBigTag(int tagNr, const char * hex)
{
	bool retval = true;
	BERSerializer ser;
	for (int i = 0 ; i < tagNr - 1 ; ++i) ser << Placeholder();
	ser << 0x42;
	const QByteArray expected = QByteArray::fromHex(hex) + QByteArray::fromHex("0142");
	if (ser.data() != expected) {
		out << "serialized hex differs:" << endl
			<< "is       : " << ser.data().toHex() << endl
			<< "expected : " << expected  .toHex() << endl;
		retval = false;
	}
	BERDeserializer deser(expected);
	for (int i = 0 ; i < tagNr - 1 ; ++i) deser >> Placeholder();
	int test; deser >> test;
	if (test != 0x42) {
		out << "deserialized values differ:" << endl
			<< "is       : " << test << endl
			<< "expected : " << 0x42 << endl;
		return false;
	}
	return retval;
}

}

class SerializeBER_Test: public QObject
{
	Q_OBJECT
private slots:

	void integer()
	{
		QVERIFY(checkSerDeser<bool   >( false, ""          ));
		QVERIFY(checkSerDeser<bool   >(  true, "c10101"    ));
		QVERIFY(checkSerDeser<quint8 >(     0, ""          ));
		QVERIFY(checkSerDeser<quint8 >(     1, "c10101"    ));
		QVERIFY(checkSerDeser<quint8 >(   127, "c1017f"    ));
		QVERIFY(checkSerDeser<quint8 >(   255, "c10200ff"  ));
		QVERIFY(checkSerDeser< qint8 >(     0, ""          ));
		QVERIFY(checkSerDeser< qint8 >(     1, "c10101"    ));
		QVERIFY(checkSerDeser< qint8 >(   127, "c1017f"    ));
		QVERIFY(checkSerDeser< qint8 >(    -1, "c101ff"    ));
		QVERIFY(checkSerDeser< qint8 >(    -2, "c101fe"    ));
		QVERIFY(checkSerDeser< qint8 >(  -128, "c10180"    ));
		QVERIFY(checkSerDeser<quint16>(     0, ""          ));
		QVERIFY(checkSerDeser<quint16>(     1, "c10101"    ));
		QVERIFY(checkSerDeser<quint16>( 32767, "c1027fff"  ));
		QVERIFY(checkSerDeser<quint16>( 65535, "c10300ffff"));
		QVERIFY(checkSerDeser< qint16>(     0, ""          ));
		QVERIFY(checkSerDeser< qint16>(     1, "c10101"    ));
		QVERIFY(checkSerDeser< qint16>(   127, "c1017f"    ));
		QVERIFY(checkSerDeser< qint16>(   128, "c1020080"  ));
		QVERIFY(checkSerDeser< qint16>( 32767, "c1027fff"  ));
		QVERIFY(checkSerDeser< qint16>(    -1, "c101ff"    ));
		QVERIFY(checkSerDeser< qint16>(    -2, "c101fe"    ));
		QVERIFY(checkSerDeser< qint16>(  -128, "c10180"    ));
		QVERIFY(checkSerDeser< qint16>(  -129, "c102ff7f"  ));
		QVERIFY(checkSerDeser< qint16>(-32767, "c1028001"  ));
		QVERIFY(checkSerDeser< qint16>(-32768, "c1028000"  ));
		QVERIFY(checkSerDeser<quint64>(     0, ""          ));
		QVERIFY(checkSerDeser<quint64>(     1, "c10101"    ));
		QVERIFY(checkSerDeser<quint64>(   128, "c1020080"  ));
		QVERIFY(checkSerDeser< qint64>(    -1, "c101ff"    ));
		QVERIFY(checkSerDeser< qint64>(    -2, "c101fe"    ));
		QVERIFY(checkSerDeser< qint64>(  -128, "c10180"    ));
		QVERIFY(checkSerDeser< qint64>(  -129, "c102ff7f"  ));

		QVERIFY(checkSerDeser<quint64>(Q_UINT64_C(0x7fffffffffffffff), "c1087fffffffffffffff"));
		QVERIFY(checkSerDeser<quint64>(Q_UINT64_C(0x8000000000000000), "c109008000000000000000"));
		QVERIFY(checkSerDeser<quint64>(Q_UINT64_C(0xfffffffffffffffe), "c10900fffffffffffffffe"));
		QVERIFY(checkSerDeser<quint64>(Q_UINT64_C(0xffffffffffffffff), "c10900ffffffffffffffff"));

		QVERIFY(checkSerDeser<qint64 >(Q_INT64_C(   36028797018963967),   "c1077fffffffffffff"));
		QVERIFY(checkSerDeser<qint64 >(Q_INT64_C(   36028797018963968),   "c1080080000000000000"));
		QVERIFY(checkSerDeser<qint64 >(Q_INT64_C( 9223372036854775807),   "c1087fffffffffffffff"));
		QVERIFY(checkSerDeser<qint64 >(Q_INT64_C(-9223372036854775807),   "c1088000000000000001"));
		QVERIFY(checkSerDeser<qint64 >(Q_INT64_C(-9223372036854775807)-1, "c1088000000000000000"));
	}

	void nullInList()
	{
		QVERIFY(checkSerDeser<QList<bool   > >(QList<bool   >() << false, "e103c08100"));
		QVERIFY(checkSerDeser<QList<quint8 > >(QList<quint8 >() << 0,     "e103c08100"));
		QVERIFY(checkSerDeser<QList< qint8 > >(QList< qint8 >() << 0,     "e103c08100"));
		QVERIFY(checkSerDeser<QList<quint16> >(QList<quint16>() << 0,     "e103c08100"));
		QVERIFY(checkSerDeser<QList<quint64> >(QList<quint64>() << 0,     "e103c08100"));

		QVERIFY(checkSer<QList<const char *>    >(QList<const char *>() << 0,            "e103c08100"));
		QVERIFY(checkSerDeser<QList<QByteArray> >(QList<QByteArray  >() << QByteArray(), "e103c08100"));
		QVERIFY(checkSerDeser<QList<QByteArray> >(QList<QByteArray  >() << QByteArray(), "e103c08100"));
		QVERIFY(checkSerDeser<QList<QString>    >(QList<QString     >() << QString(),    "e103c08100"));
		QVERIFY(checkSerDeser<QList<QChar>      >(QList<QChar       >() << QChar(),      "e103c08100"));
	}

	void string()
	{
		QVERIFY(checkSer<const char *>(0,    ""));
		QVERIFY(checkSer<const char *>("",   "c100"));
		QVERIFY(checkSer<const char *>("X",  "c10158"));
		QVERIFY(checkSer<const char *>("XY", "c1025859"));

		QVERIFY(checkSerDeserNull<QByteArray>(QByteArray(),     ""));
		QVERIFY(checkSerDeserNull<QByteArray>(QByteArray(""),   "c100"));
		QVERIFY(checkSerDeserNull<QByteArray>(QByteArray("X"),  "c10158"));
		QVERIFY(checkSerDeserNull<QByteArray>(QByteArray("XY"), "c1025859"));
		QVERIFY(checkSerDeserNull<QByteArray>(QByteArray::fromHex("003132"), "c103 003132"));
		QVERIFY(checkSerDeserNull<QByteArray>(QByteArray::fromHex("310032"), "c103 310032"));
		QVERIFY(checkSerDeserNull<QByteArray>(QByteArray::fromHex("313200"), "c103 313200"));

		QVERIFY(checkSerDeserNull<QString>(QString(),     ""));
		QVERIFY(checkSerDeserNull<QString>(QString(""),   "c100"));
		QVERIFY(checkSerDeserNull<QString>(QString("X"),  "c10158"));
		QVERIFY(checkSerDeserNull<QString>(QString("XY"), "c1025859"));
		QVERIFY(checkSerDeserNull<QString>(QString("XäÄöÖüÜßY"), "c1 10 58 c3a4 c384 c3b6 c396 c3bc c39c c39f 59"));

		QVERIFY(checkSerDeserNull<QChar>(QChar(),         ""));
		QVERIFY(checkSerDeserNull<QChar>(QString("A")[0], "c10141"));
		QVERIFY(checkSerDeserNull<QChar>(QString("ä")[0], "c10200e4"));
		QVERIFY(checkSerDeserNull<QChar>(QString("ß")[0], "c10200df"));
	}

	void many()
	{
		{
			BERSerializer ser;
			ser << 17 << 18 << 0 << 20;
			const QByteArray hex = QByteArray::fromHex("C10111 C20112        C40114");
			QCOMPARE(ser.data(), hex);
			BERDeserializer deser(hex);
			int a, b, c, d; deser >> a >> b >> c >> d;
			QCOMPARE(a, 17);
			QCOMPARE(b, 18);
			QCOMPARE(c,  0);
			QCOMPARE(d, 20);
		}{
			BERSerializer ser;
			ser << 17 << Placeholder() << 19;
			const QByteArray hex = QByteArray::fromHex("C10111        C30113");
			QCOMPARE(ser.data(), hex);
			BERDeserializer deser(hex);
			int a, b; deser >> a >> Placeholder() >> b;
			QCOMPARE(a, 17);
			QCOMPARE(b, 19);
		}{
			const QByteArray hex = QByteArray::fromHex("C10111 C20112 C30113");
			BERDeserializer deser(hex);
			int a, c; deser >> a >> Placeholder() >> c;
			QCOMPARE(a, 17);
			QCOMPARE(c, 19);
		}
	}

	void bigTag()
	{
		QVERIFY(testBigTag(   30, "DE"));
		QVERIFY(testBigTag(   31, "DF1F"));
		QVERIFY(testBigTag(  127, "DF7f"));
		QVERIFY(testBigTag(  128, "DF8100"));
		QVERIFY(testBigTag(  129, "DF8101"));
		QVERIFY(testBigTag(  255, "DF817f"));
		QVERIFY(testBigTag(  256, "DF8200"));
		QVERIFY(testBigTag(16383, "DFff7f"));
		QVERIFY(testBigTag(16384, "DF818000"));
	}

	void object()
	{
		Test2 t2;
		t2.t1.a = 0x42;
		t2.t1.b = 0x23;
		t2.a = 0x43;
		QVERIFY(checkSerDeser<Test2>(t2, "e1 0b e1 06 c1 01 42 c2 01 23 c2 01 43"));
		t2.t1.a = 0;
		t2.t1.b = 0;
		QVERIFY(checkSerDeser<Test2>(t2, "E1 03 C2 01 43"));
		t2.a = 0;
		QVERIFY(checkSerDeser<Test2>(t2, ""));
		t2.t1.a = 0x42;
		QVERIFY(checkSerDeser<Test2>(t2, "e1 05 e1 03 c1 01 42"));
	}

	void lists()
	{
		QVERIFY(checkSerDeser<QList<quint8 > >(QList<quint8 >(), ""));
		QVERIFY(checkSerDeser<QList<quint8 > >(QList<quint8 >() << 0x42, "e103c00142"));
		QVERIFY(checkSerDeser<QList<quint8 > >(QList<quint8 >() << 0x42 << 0x43, "e106c00142c00143"));
		QVERIFY(checkSerDeser<QStringList    >(QStringList() << QString("XY") << QString() << "" << "A",
			"e10c c0025859 c08100 c000 c00141"));

		BERDeserializer ser(QByteArray::fromHex("e105 c08100 c000"));
		QStringList list; ser >> list;
		QVERIFY(list[0].isNull());
		QVERIFY(!list[1].isNull());
		QVERIFY(list[1].isEmpty());
	}

	void maps()
	{
		typedef QMap<QString, int> Map;
		Map map;
		QVERIFY(checkSerDeser<Map>(map, ""));
		map["xy"] = 4;
		QVERIFY(checkSerDeser<Map>(map, "e107 c0027879 c00104"));
		map["xyz"] = 7;
		QVERIFY(checkSerDeser<Map>(map, "e10f c00378797a c00107 c0027879 c00104"));
	}

};
#include "serializeber_test.moc"
ADD_TEST(SerializeBER_Test)

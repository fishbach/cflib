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

#include <cflib/serialize/serialize.h>
#include <cflib/serialize/serializejs.h>
#include <cflib/serialize/serialize_test/test.h>
#include <cflib/util/test.h>

using namespace cflib::serialize;

namespace {

QTextStream out(stdout);

template <typename T>
bool checkSer(T val, const QByteArray & expected) {
	JSSerializer ser;
	ser << val;
	if (ser.data() != expected) {
		out << "serialized hex differs:" << endl
			<< "is       : " << ser.data() << endl
			<< "expected : " << expected   << endl;
		return false;
	}
	return true;
}

template <typename T>
bool checkDeser(T val, const QByteArray & expected) {
	JSDeserializer deser(expected);
	T test; deser >> test;
	if (test != val) {
		out << "deserialized values differ:" << endl
			<< "is       : " << test << endl
			<< "expected : " << val  << endl;
		return false;
	}
	return true;
}

template <typename T>
bool checkDeserNull(T val, const QByteArray & expected) {
	JSDeserializer deser(expected);
	T test; deser >> test;
	if (test.isNull() != val.isNull()) {
		out << "deserialized isNull differs:" << endl
			<< "is       : " << test.isNull() << endl
			<< "expected : " << val.isNull()  << endl;
		return false;
	}
	return true;
}

template <typename T>
bool checkSerDeser(T val, const QByteArray & expected) {
	return checkSer(val, expected) & checkDeser(val, expected);
}

template <typename T>
bool checkSerDeserNull(T val, const QByteArray & expected) {
	return checkSer(val, expected) & checkDeser(val, expected) & checkDeserNull(val, expected);
}

}

class SerializeJS_Test: public QObject
{
	Q_OBJECT
private slots:

	void integer() {
		QVERIFY(checkSerDeser<bool   >( false, "[,]" ));
		QVERIFY(checkSerDeser<bool   >(  true, "[1]" ));
		QVERIFY(checkSerDeser<quint8 >(     0, "[,]" ));
		QVERIFY(checkSerDeser<quint8 >(     1, "[1]" ));
		QVERIFY(checkSerDeser< qint8 >(    -1, "[-1]" ));
		QVERIFY(checkSerDeser<quint64>(  1234, "[1234]"));
		QVERIFY(checkSerDeser< qint64>( -1234, "[-1234]"));

		QVERIFY(checkSerDeser<quint64>(Q_UINT64_C(0x7fffffffffffffff), "[9223372036854775807]"));
		QVERIFY(checkSerDeser<quint64>(Q_UINT64_C(0x8000000000000000), "[9223372036854775808]"));
		QVERIFY(checkSerDeser<quint64>(Q_UINT64_C(0xfffffffffffffffe), "[18446744073709551614]"));
		QVERIFY(checkSerDeser<quint64>(Q_UINT64_C(0xffffffffffffffff), "[18446744073709551615]"));

		QVERIFY(checkSerDeser<qint64 >(Q_INT64_C(   36028797018963967),   "[36028797018963967]"));
		QVERIFY(checkSerDeser<qint64 >(Q_INT64_C(   36028797018963968),   "[36028797018963968]"));
		QVERIFY(checkSerDeser<qint64 >(Q_INT64_C( 9223372036854775807),   "[9223372036854775807]"));
		QVERIFY(checkSerDeser<qint64 >(Q_INT64_C(-9223372036854775807),   "[-9223372036854775807]"));
		QVERIFY(checkSerDeser<qint64 >(Q_INT64_C(-9223372036854775807)-1, "[-9223372036854775808]"));
	}

	void null() {
		QVERIFY(checkSerDeser<bool   >(false, "[,]"));
		QVERIFY(checkSerDeser<quint8 >(    0, "[,]"));
		QVERIFY(checkSerDeser< qint8 >(    0, "[,]"));
		QVERIFY(checkSerDeser<quint16>(    0, "[,]"));
		QVERIFY(checkSerDeser<quint64>(    0, "[,]"));
	}

	void string() {
		QVERIFY(checkSer<const char *>(0,    "[,]"));
		QVERIFY(checkSer<const char *>("",   "['']"));
		QVERIFY(checkSer<const char *>("X",  "['X']"));
		QVERIFY(checkSer<const char *>("XY", "['XY']"));
		QVERIFY(checkSer<const char *>("X\\r\r\n", "['X\\\\r\\r\\n']"));

		QVERIFY(checkSerDeserNull<QByteArray>(QByteArray(),     "[,]"));
		QVERIFY(checkSerDeserNull<QByteArray>(QByteArray(""),   "['']"));
		QVERIFY(checkSerDeserNull<QByteArray>(QByteArray("X"),  "['X']"));
		QVERIFY(checkSerDeserNull<QByteArray>(QByteArray("XY"), "['XY']"));
		QVERIFY(checkSerDeserNull<QByteArray>(QByteArray("X\\r\r\n"), "['X\\\\r\\r\\n']"));
		QVERIFY(checkSerDeserNull<QByteArray>(QByteArray::fromHex("003132"), QByteArray("['") + '\0' + "12']"));
		QVERIFY(checkSerDeserNull<QByteArray>(QByteArray::fromHex("310032"), QByteArray("['1") + '\0' + "2']"));
		QVERIFY(checkSerDeserNull<QByteArray>(QByteArray::fromHex("313200"), QByteArray("['12") + '\0' + "']"));

		QVERIFY(checkSerDeserNull<QString>(QString(),     "[,]"));
		QVERIFY(checkSerDeserNull<QString>(QString(""),   "['']"));
		QVERIFY(checkSerDeserNull<QString>(QString("X"),  "['X']"));
		QVERIFY(checkSerDeserNull<QString>(QString("XY"), "['XY']"));
		QVERIFY(checkSerDeserNull<QString>(QString("X\\r\r\n"), "['X\\\\r\\r\\n']"));
		QVERIFY(checkSerDeserNull<QString>(QString("XäÄöÖüÜßY"), "['XäÄöÖüÜßY']"));

		QVERIFY(checkSerDeserNull<QString>(QChar(QChar::LineSeparator), "['\\u2028']"));
		QVERIFY(checkSerDeserNull<QString>(
			QString("X") + QChar(QChar::LineSeparator) + "Y" + QChar(QChar::ParagraphSeparator) + "\\u1234Z",
			"['X\\u2028Y\\u2029\\\\u1234Z']"));

		// this is broken input
		QVERIFY(checkDeser<QString>(QString("A\\u1234\\x\\B"), "['A\\u1234\\x\\\\B']"));
	}

	void many() {
		{
			JSSerializer ser;
			ser << 17 << 18 << 0 << 20;
			const QByteArray data = "[17,18,,20]";
			QCOMPARE(ser.data(), data);
			JSDeserializer deser(data);
			int a, b, c, d; deser >> a >> b >> c >> d;
			QCOMPARE(a, 17);
			QCOMPARE(b, 18);
			QCOMPARE(c,  0);
			QCOMPARE(d, 20);
		}{
			JSSerializer ser;
			ser << 17 << Placeholder() << 19;
			const QByteArray data = "[17,,19]";
			QCOMPARE(ser.data(), data);
			JSDeserializer deser(data);
			int a, b; deser >> a >> Placeholder() >> b;
			QCOMPARE(a, 17);
			QCOMPARE(b, 19);
		}{
			const QByteArray data = "[17,18,19]";
			JSDeserializer deser(data);
			int a, c; deser >> a >> Placeholder() >> c;
			QCOMPARE(a, 17);
			QCOMPARE(c, 19);
		}{
			const QByteArray data = "[17,[1,2,3,[4,5]],19]";
			JSDeserializer deser(data);
			int a, c; deser >> a >> Placeholder() >> c;
			QCOMPARE(a, 17);
			QCOMPARE(c, 19);
		}
	}

	void object() {
		Test2 t2;
		t2.t1.a = 0x42;
		t2.t1.b = 0x23;
		t2.a = 0x43;
		QVERIFY(checkSerDeser<Test2>(t2, "[[[66,35],67]]"));
		t2.t1.a = 0;
		t2.t1.b = 0;
		QVERIFY(checkSerDeser<Test2>(t2, "[[,67]]"));
		t2.a = 0;
		QVERIFY(checkSerDeser<Test2>(t2, "[[,,]]"));
		t2.t1.a = 0x42;
		QVERIFY(checkSerDeser<Test2>(t2, "[[[66,,],,]]"));
	}

	void lists()
	{
		QVERIFY(checkSerDeser<QList<quint8 > >(QList<quint8 >(), "[[]]"));
		QVERIFY(checkSerDeser<QList<quint8 > >(QList<quint8 >() << 0x42, "[[66]]"));
		QVERIFY(checkSerDeser<QList<quint8 > >(QList<quint8 >() << 0x42 << 0x43, "[[66,67]]"));
		QVERIFY(checkSerDeser<QStringList    >(QStringList() << QString("XY") << QString() << "" << "A",
			"[['XY',,'','A']]"));

		JSDeserializer ser("[[,'']]");
		QStringList list; ser >> list;
		QVERIFY(list[0].isNull());
		QVERIFY(!list[1].isNull());
		QVERIFY(list[1].isEmpty());
	}

	void maps()
	{
		{
			typedef QMap<int, int> Map;
			Map map;
			QVERIFY(checkSerDeser<Map>(map, "[[]]"));
			map[13] = 4;
			QVERIFY(checkSerDeser<Map>(map, "[[13,4]]"));
			map[157] = 7;
			QVERIFY(checkSerDeser<Map>(map, "[[157,7,13,4]]"));
		}
		{
			typedef QMap<QString, int> Map;
			Map map;
			QVERIFY(checkSerDeser<Map>(map, "[[]]"));
			map["xy"] = 4;
			QVERIFY(checkSerDeser<Map>(map, "[['xy',4]]"));
			map["xyz"] = 7;
			QVERIFY(checkSerDeser<Map>(map, "[['xyz',7,'xy',4]]"));
		}
	}

	void someStrings()
	{
		JSSerializer ser;
		ser << Placeholder() << "bla" << "dudi" << Placeholder();
		QCOMPARE(ser.data(), QByteArray("[,'bla','dudi',,]"));
		{
			JSDeserializer deser("[,'bla','dudi',]");
			QString a, b;
			deser >> Placeholder() >> a >> b >> Placeholder();
			QCOMPARE(a, QString("bla"));
			QCOMPARE(b, QString("dudi"));
		}
		{
			JSDeserializer deser("[,'bla','dudi']");
			QString a, b;
			deser >> Placeholder() >> a >> b >> Placeholder();
			QCOMPARE(a, QString("bla"));
			QCOMPARE(b, QString("dudi"));
		}
		{
			JSDeserializer deser("[,'bla',]");
			QString a, b;
			deser >> Placeholder() >> a >> b >> Placeholder();
			QCOMPARE(a, QString("bla"));
			QCOMPARE(b, QString());
		}
		{
			JSDeserializer deser("[,'bla']");
			QString a, b;
			deser >> Placeholder() >> a >> b >> Placeholder();
			QCOMPARE(a, QString("bla"));
			QCOMPARE(b, QString());
		}
		{
			JSDeserializer deser("[,'bl,a']");
			QString a, b;
			deser >> Placeholder() >> a >> b >> Placeholder();
			QCOMPARE(a, QString("bl,a"));
			QCOMPARE(b, QString());
		}
		{
			JSDeserializer deser("[,'bl,a','du,di']");
			QString a, b;
			deser >> Placeholder() >> a >> b >> Placeholder();
			QCOMPARE(a, QString("bl,a"));
			QCOMPARE(b, QString("du,di"));
		}
	}

};
#include "serializejs_test.moc"
ADD_TEST(SerializeJS_Test)

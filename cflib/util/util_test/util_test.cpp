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

namespace {

QTextStream out(stdout);

bool checkDeflate(const QByteArray & source, const QByteArray & enc, int level)
{
	QByteArray data = source;
	deflateRaw(data, level);
	if (data != QByteArray::fromHex(enc)) {
		out << "compressed differs: " << endl
			<< "is: " << data.toHex() << endl
			<< "ex: " << enc << endl;
		return false;
	}
	inflateRaw(data);
	if (data != source) {
		out << "decompressed differs: " << endl
			<< "is: " << data.toHex() << endl
			<< "ex: " << source.toHex() << endl;
		return false;
	}
	return true;
}

}

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

	void test_QString()
	{
		QVERIFY(QString().isNull());
		QVERIFY(!QString("").isNull());
	}

	void test_QByteArray()
	{
		QByteArray ba;
		QVERIFY(ba.isNull());
		QCOMPARE(ba.capacity(), 0);
		ba.reserve(997);
		QCOMPARE(ba.capacity(), 997);
		ba.resize(123);
		QCOMPARE(ba.size(), 123);
		QCOMPARE(ba.capacity(), 997);
		ba.resize(0);
		QVERIFY(!ba.isNull());
		QCOMPARE(ba.size(), 0);
		QCOMPARE(ba.capacity(), 997);
		ba.clear();
		QVERIFY(ba.isNull());
		QCOMPARE(ba.capacity(), 0);
		QVERIFY(!QByteArray("").isNull());
	}

	void test_gzip()
	{
		const QByteArray lorem =
			"Lorem ipsum dolor sit amet, consetetur sadipscing elitr, "
			"sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, "
			"sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum.";
		QByteArray data = lorem;
		data.detach();
		gzip(data, 1);
		QCOMPARE(data, QByteArray::fromHex(
			"1f8b08000000000000ff3d8e410ec2300c04bfb20fa8f803773e619a556514c725b12bf5f7a42071f0c55ecfecc33b"
			"0dba8f3414afde313420c658b07a1b0c46cea5949959b56d60d5e80b060b8a8aa1794b3b41ede60541db2744dba125"
			"5b2003559e5302c64f40986c4d2055df794e00bb4cd99f7778cd3d526eb8070e76077d5cdfb2ae39ae7ce095231c25"
			"e75c9df9bd53d0f94cbb7d00a1dac561d4000000"));

		data = lorem;
		data.detach();
		gzip(data, 9);
		QCOMPARE(data, QByteArray::fromHex(
			"1f8b08000000000000ff4d8d4b0ec2300c05aff20e507107f65cc24dacca28b64b6257eaed498105dbf79979786785"
			"ec2315d59b770c0990722c286e8383236748756e8ad8066e127dc1e08a2aa430b7d4132c5dbd2258f709113ba4a605"
			"32d0689d12707c050ca5cd08d4e495e70470a7f8e31dde728fa41bee8183bb837d5c6f2a25c7b50f3c7384a3a6ff90"
			"9f9e099dd7d4db1ba1dac561d4000000"));
	}

	void test_deflate()
	{
		QVERIFY(checkDeflate(QByteArray(),        "00",                     1));
		QVERIFY(checkDeflate(QByteArray(),        "00",                     0));
		QVERIFY(checkDeflate(QByteArray("\0", 1), "620000",                 1));
		QVERIFY(checkDeflate("A",                 "720400",                 1));
		QVERIFY(checkDeflate("A",                 "000100feff4100",         0));
		QVERIFY(checkDeflate("bc",                "4a4a0600",               1));
		QVERIFY(checkDeflate("Hello",             "f248cdc9c90700",         1));
		QVERIFY(checkDeflate("Hello",             "000500faff48656c6c6f00", 0));
		QByteArray data;
		inflateRaw(data);
		QVERIFY(data.isEmpty());
		data = "\x00";
		inflateRaw(data);
		QVERIFY(data.isEmpty());
	}

};
#include "util_test.moc"
ADD_TEST(Util_Test)

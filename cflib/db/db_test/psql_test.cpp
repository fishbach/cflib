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

#include <cflib/db/psql.h>
#include <cflib/util/test.h>

using namespace cflib::db;

USE_LOG(LogCat::Db)

class PSql_test : public QObject
{
	Q_OBJECT
private slots:

	void initTestCase()
	{
		QVERIFY(PSql::setParameter("host=127.0.0.1 port=5432"));
		PSqlConn;

		// drop any old exisiting table
		sql.exec("DROP TABLE cflib_db_test");

		QVERIFY(sql.exec(
			"CREATE TABLE cflib_db_test ("
				"id integer NOT NULL, "
				"x16 smallint, "
				"x32 integer, "
				"x64 bigint, "
				"t timestamp, "
				"a bytea, "
				"s text, "
				"r real, "
				"d double precision, "
				"PRIMARY KEY (id)"
			")"));
	}

	void cleanupTestCase()
	{
		PSqlConn;
//		QVERIFY(sql.exec("DROP TABLE cflib_db_test"));
	}

	void simple_test()
	{
		PSqlConn;

		QVERIFY( sql.exec("SELECT 42"));
		QVERIFY( sql.next());
		QVERIFY(!sql.next());

		QVERIFY( sql.exec("SELECT 23;"));
		QVERIFY( sql.next());

		QVERIFY(!sql.exec("SULICT 42"));
		QVERIFY(!sql.next());
	}

	void table_test()
	{
		PSqlConn;

		QVERIFY(sql.exec(
			"INSERT INTO "
				"cflib_db_test "
			"("
				"id, x16, x32, x64, t, a, s, r, d"
			") VALUES ("
				"1, 2, 3, 4, '2017-02-27T14:47:34.123Z', E'\\x41\\x30', E'ABC\\xC3\\xB6\\xC3\\x9F', 1.23, 3.45"
			")"
		));
		QVERIFY(!sql.next());

		QVERIFY(sql.exec(
			"INSERT INTO "
				"cflib_db_test "
			"("
				"id, x16, x32, x64, t, a, s, r, d"
			") VALUES ("
				"2, 5, 6, 7, '2017-01-12T11:17:24.253Z', '', '', 0, 'NaN'"
			")"
		));

		sql.prepare(
			"INSERT INTO "
				"cflib_db_test "
			"("
				"id, x16, x32, x64, t"
			") VALUES ("
				"$1, $2, $3, $4, $5"
			")"
		);
		sql << 3 << 45 << 67 << 89 << QDateTime(QDate(2016, 2, 27), QTime(10, 47, 34, 123), Qt::UTC);
		QVERIFY(sql.exec());
		sql << 4 << (qint8)-45 << sql.null << (qint32)-89 << sql.null;
		QVERIFY(sql.exec());

		QVERIFY(sql.exec("SELECT id, x16, x32, x64, t, a, s, r, d FROM cflib_db_test"));

		quint32 id;
		quint16 x16;
		quint32 x32;
		quint64 x64;
		QDateTime t;
		QByteArray a;
		QString s;
		float f;
		double d;

		QVERIFY(sql.next());
		sql >> id >> x16 >> x32 >> x64 >> t >> a >> s >> f >> d;
		QCOMPARE(id,  (quint32)1);
		QCOMPARE(x16, (quint16)2);
		QCOMPARE(x32, (quint32)3);
		QCOMPARE(x64, (quint64)4);
		QCOMPARE(t, QDateTime(QDate(2017, 2, 27), QTime(14, 47, 34, 123), Qt::UTC));
		QCOMPARE(a, QByteArray("A0"));
		QCOMPARE(s, QString::fromUtf8("ABC\xC3\xB6\xC3\x9F"));
		QVERIFY(qFuzzyCompare(f, 1.23f));
		QVERIFY(qFuzzyCompare(d, 3.45));

		QVERIFY(sql.next());
		sql >> id >> x16 >> sql.null >> x64 >> t >> a >> s >> f >> d;
		QCOMPARE(id,  (quint32)2);
		QCOMPARE(x16, (quint16)5);
		QCOMPARE(x64, (quint64)7);
		QCOMPARE(t, QDateTime(QDate(2017, 1, 12), QTime(11, 17, 24, 253), Qt::UTC));
		QCOMPARE(a, QByteArray(""));
		QCOMPARE(s, QString(""));
		QCOMPARE(f, 0.0f);
		QVERIFY(isnan(d));

		QVERIFY(sql.next());
		QVERIFY(!sql.isNull());
		sql >> id >> x16 >> x32 >> x64;
		QCOMPARE(id,  (quint32)3);
		QCOMPARE(x16, (quint16)45);
		QCOMPARE(x32, (quint32)67);
		QCOMPARE(x64, (quint64)89);
		QVERIFY(!sql.isNull());
		sql >> t;
		QCOMPARE(t, QDateTime(QDate(2016, 2, 27), QTime(10, 47, 34, 123), Qt::UTC));
		QVERIFY(!sql.lastFieldIsNull());
		QVERIFY(sql.isNull());
		sql >> sql.null;
		QVERIFY(sql.lastFieldIsNull());
		QVERIFY(sql.isNull());
		sql >> sql.null;
		QVERIFY(sql.lastFieldIsNull());
		QVERIFY(sql.isNull());
		sql >> sql.null;
		QVERIFY(sql.lastFieldIsNull());
		QVERIFY(sql.isNull());
		sql >> sql.null;
		QVERIFY(sql.lastFieldIsNull());

		qint16 sx16;
		qint32 sx32;
		qint64 sx64;

		QVERIFY(sql.next());
		QVERIFY(!sql.isNull(1));
		QVERIFY( sql.isNull(2));
		sql >> id >> sx16;
		QCOMPARE(id,  (quint32)4);
		QCOMPARE(sx16, (qint16)-45);
		QVERIFY(sql.isNull());
		sql >> sx32;
		QVERIFY(sql.lastFieldIsNull());
		sql >> sx64;
		QCOMPARE(sx64, (qint64)-89);

		QVERIFY(!sql.next());
	}

	void transaction_test()
	{
		PSqlConn;

		QVERIFY(!sql.commit());

		sql.begin();
		QVERIFY(sql.commit());

		sql.begin();
		sql.rollback();
		QVERIFY(!sql.commit());

		sql.begin();
		{
			PSqlConn;

			QVERIFY(!sql.commit());

			sql.begin();
			QVERIFY(sql.commit());
		}
		QVERIFY(sql.commit());

		sql.begin();
		{
			PSqlConn;

			sql.begin();
			sql.rollback();
			QVERIFY(!sql.commit());
		}
		QVERIFY(!sql.commit());
	}

	void select_test()
	{
		PSqlConn;

	}

};
#include "psql_test.moc"
ADD_TEST(PSql_test)

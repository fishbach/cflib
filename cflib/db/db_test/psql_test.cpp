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

#include <cmath>

using namespace cflib::db;

USE_LOG(LogCat::Db)

class PSql_test : public QObject
{
	Q_OBJECT
private:

	struct TestTypes
	{
		quint32 id;
		quint16 x16;
		quint32 x32;
		quint64 x64;
		QDateTime t;
		QByteArray a;
		QString s;
		float f;
		double d;
	};
	TestTypes tt;

private slots:

	void initTestCase()
	{
		QVERIFY(PSql::setParameter("host=127.0.0.1 port=5432"));
		PSqlConn;

		// drop any old exisiting table
		sql.exec("DROP TABLE cflib_db_test");
		sql.exec("DROP TABLE cflib_db_test_2");
		sql.exec("DROP TABLE cflib_db_test_3");

		QVERIFY(sql.exec(
			"CREATE TABLE cflib_db_test ("
				"id integer NOT NULL, "
				"x16 smallint, "
				"x32 integer, "
				"x64 bigint, "
				"t timestamp with time zone, "
				"a bytea, "
				"s text, "
				"r real, "
				"d double precision, "
				"PRIMARY KEY (id)"
			")"));
		QVERIFY(sql.exec(
			"CREATE TABLE cflib_db_test_2 ("
				"id serial NOT NULL, "
				"x32 integer, "
				"PRIMARY KEY (id)"
			")"));
		QVERIFY(sql.exec(
			"CREATE TABLE cflib_db_test_3 ("
				"id bigserial NOT NULL, "
				"x32 integer, "
				"PRIMARY KEY (id)"
			")"));
	}

	void cleanupTestCase()
	{
		PSqlConn;
		QVERIFY(sql.exec("DROP TABLE cflib_db_test"));
		QVERIFY(sql.exec("DROP TABLE cflib_db_test_2"));
		QVERIFY(sql.exec("DROP TABLE cflib_db_test_3"));
	}

	// -----------------------------------------------------------

	void result_test()
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

	// -----------------------------------------------------------

	void insert_test_01()
	{
		PSqlConn;

		QVERIFY(sql.exec(
			"INSERT INTO "
				"cflib_db_test "
			"("
				"id, x16, x32, x64, t, a, s, r, d"
			") VALUES ("
				"1, 2, -1, 4, '2017-02-27T14:47:34.123Z', E'\\x41\\x30', E'ABC\\xC3\\xB6\\xC3\\x9F', 1.23, 3.45"
			")"
		));
	}

	void select_test_01()
	{
		PSqlConn;

		QVERIFY(sql.exec("SELECT id, x16, x32, x64, t, a, s, r, d FROM cflib_db_test"));

		QVERIFY(sql.next());
		sql >> tt.id >> tt.x16 >> tt.x32 >> tt.x64 >> tt.t >> tt.a >> tt.s >> tt.f >> tt.d;
		QCOMPARE(tt.id,  (quint32)1);
		QCOMPARE(tt.x16, (quint16)2);
		QCOMPARE(tt.x32, (quint32)0xFFFFFFFF);
		QCOMPARE(tt.x64, (quint64)4);
		QCOMPARE(tt.t, QDateTime(QDate(2017, 2, 27), QTime(14, 47, 34, 123), Qt::UTC));
		QCOMPARE(tt.a, QByteArray("A0"));
		QCOMPARE(tt.s, QString::fromUtf8("ABC\xC3\xB6\xC3\x9F"));
		QVERIFY(qFuzzyCompare(tt.f, 1.23f));
		QVERIFY(qFuzzyCompare(tt.d, 3.45));
		// no futher lines
		QVERIFY(!sql.next());
	}

	void clean_test_01()
	{
		// leave table empty
		PSqlConn;
		QVERIFY(sql.exec("DELETE FROM cflib_db_test WHERE id=1;"));
	}

	// -----------------------------------------------------------

	void insert_test_02()
	{
		PSqlConn;
		QVERIFY(sql.exec(
			"INSERT INTO "
				"cflib_db_test "
			"("
				"id, x16, x32, x64, t, a, s, r, d"
			") VALUES ("
				"2, 5, 6, 7, CURRENT_TIMESTAMP, '', '', 0, 'NaN'"
			")"
		));
	}

	void select_test_02()
	{
		PSqlConn;

		QVERIFY(sql.exec("SELECT id, x16, x32, x64, t, a, s, r, d FROM cflib_db_test"));

		QVERIFY(sql.next());
		sql >> tt.id >> tt.x16 >> tt.x32 >> tt.x64 >> tt.t >> tt.a >> tt.s >> tt.f >> tt.d;
		QCOMPARE(tt.id,  (quint32)2);
		QCOMPARE(tt.x16, (quint16)5);
		QCOMPARE(tt.x64, (quint64)7);
		QVERIFY(QDateTime::currentDateTimeUtc().addSecs(-30) < tt.t);
		QVERIFY(QDateTime::currentDateTimeUtc().addSecs( 30) > tt.t);
		QCOMPARE(tt.a, QByteArray(""));
		QCOMPARE(tt.s, QString(""));
		QCOMPARE(tt.f, 0.0f);
		QVERIFY(std::isnan(tt.d));
		// no further lines
		QVERIFY(!sql.next());
	}

	void clean_test_02()
	{
		// leave table empty
		PSqlConn;
		QVERIFY(sql.exec("DELETE FROM cflib_db_test WHERE id=2;"));
	}

	// -----------------------------------------------------------

	void serial_test()
	{
		PSqlConn;

		sql.prepare(
			"INSERT INTO "
				"cflib_db_test_2 "
			"("
				"x32"
			") VALUES ("
				"$1"
			") RETURNING id"
		);

		sql << 42;
		QVERIFY(sql.exec());
		QVERIFY(sql.next());
		sql >> tt.id;
		QCOMPARE(tt.id,  (quint32)1);

		sql << 23;
		QVERIFY(sql.exec());
		QVERIFY(sql.next());
		sql >> tt.id;
		QCOMPARE(tt.id,  (quint32)2);

		QVERIFY(sql.exec("SELECT id, x32 FROM cflib_db_test_2"));
		QVERIFY(sql.next());
		sql >> tt.id >> tt.x32;
		QCOMPARE(tt.id,  (quint32)1);
		QCOMPARE(tt.x32, (quint32)42);
		QVERIFY(sql.next());
		sql >> tt.id >> tt.x32;
		QCOMPARE(tt.id,  (quint32)2);
		QCOMPARE(tt.x32, (quint32)23);
		QVERIFY(!sql.next());
	}

	void bigserial_test()
	{
		PSqlConn;

		sql.prepare(
			"INSERT INTO "
				"cflib_db_test_3 "
			"("
				"x32"
			") VALUES ("
				"$1"
			") RETURNING id"
		);

		sql << 456;
		QVERIFY(sql.exec());
		QVERIFY(sql.next());
		sql >> tt.x64;
		QCOMPARE(tt.x64,  (quint32)1);

		sql << 765;
		QVERIFY(sql.exec());
		QVERIFY(sql.next());
		sql >> tt.x64;
		QCOMPARE(tt.x64,  (quint32)2);

		QVERIFY(sql.exec("SELECT id, x32 FROM cflib_db_test_3"));
		QVERIFY(sql.next());
		sql >> tt.x64 >> tt.x32;
		QCOMPARE(tt.x64,  (quint32)1);
		QCOMPARE(tt.x32, (quint32)456);
		QVERIFY(sql.next());
		sql >> tt.x64 >> tt.x32;
		QCOMPARE(tt.x64,  (quint32)2);
		QCOMPARE(tt.x32, (quint32)765);
		QVERIFY(!sql.next());
	}

	// -----------------------------------------------------------

	void insert_prepared_test()
	{
		PSqlConn;

		sql.prepare(
			"INSERT INTO "
				"cflib_db_test "
			"("
				"id, x16, x32, x64, t, a, s, r, d"
			") VALUES ("
				"$1, $2, $3, $4, $5, $6, $7, $8, $9"
			")"
		);
		sql << 3
			<< (quint16)0xFFFA << 67 << 89
			<< QDateTime(QDate(2016, 2, 27), QTime(10, 47, 34, 123), Qt::UTC)
			<< sql.null << sql.null
			<< 123.456f << 789.123;
		QVERIFY(sql.exec());
		sql << 4
			<< (qint8)-45 << sql.null << (qint32)-89
			<< sql.null
			<< sql.null << sql.null
			<< sql.null << sql.null;
		QVERIFY(sql.exec());
	}

	void select_prepared_insert_test()
	{
		PSqlConn;

		QVERIFY(sql.exec("SELECT id, x16, x32, x64, t, a, s, r, d FROM cflib_db_test"));

		QVERIFY(sql.next());
		QVERIFY(!sql.isNull());
		sql >> tt.id >> tt.x16 >> tt.x32 >> tt.x64;
		QCOMPARE(tt.id,  (quint32)3);
		QCOMPARE(tt.x16, (quint16)0xFFFA);
		QCOMPARE(tt.x32, (quint32)67);
		QCOMPARE(tt.x64, (quint64)89);
		QVERIFY(!sql.isNull());
		sql >> tt.t;
		QCOMPARE(tt.t, QDateTime(QDate(2016, 2, 27), QTime(10, 47, 34, 123), Qt::UTC));
		QVERIFY(!sql.lastFieldIsNull());
		sql >> tt.a >> tt.s >> tt.f >> tt.d;
		QCOMPARE(tt.f, 123.456f);
		QCOMPARE(tt.d, 789.123);
	}

	// -----------------------------------------------------------

	void select_NULLs_test()
	{
		PSqlConn;

		QVERIFY(sql.exec("SELECT id, x16, x32, x64, t, a, s, r, d FROM cflib_db_test"));
		QVERIFY(sql.next());

		sql >> sql.null;	// id
		sql >> sql.null;	// x16
		sql >> sql.null;	// x32
		sql >> sql.null;	// x64
		sql >> sql.null;	// t

		QVERIFY(sql.isNull());
		sql >> sql.null;
		QVERIFY(sql.lastFieldIsNull());
		QVERIFY(sql.isNull());
		sql >> sql.null;
		QVERIFY(sql.lastFieldIsNull());
		QVERIFY(!sql.isNull());
		sql >> sql.null;
		QVERIFY(!sql.lastFieldIsNull());

		qint16 sx16;
		qint32 sx32;
		qint64 sx64;

		QVERIFY(sql.next());
		sql >> tt.id >> sx16;
		QCOMPARE(tt.id,  (quint32)4);
		QCOMPARE(sx16, (qint16)-45);
		QVERIFY(sql.isNull());
		sql >> sx32;
		QVERIFY(sql.lastFieldIsNull());
		sql >> sx64;
		QCOMPARE(sx64, (qint64)-89);
		QVERIFY(!sql.isNull(1));
		QVERIFY( sql.isNull(2));

		QVERIFY(!sql.next());

		// leave talbe empty
		QVERIFY(sql.exec("DELETE FROM cflib_db_test WHERE id=3;"));
		QVERIFY(sql.exec("DELETE FROM cflib_db_test WHERE id=4;"));
	}

	// -----------------------------------------------------------

	void insert_special_symbols_test()
	{
		PSqlConn;

		QVERIFY(sql.exec(QString::fromUtf8(
			"INSERT INTO "
				"cflib_db_test "
			"("
				"id, x16, x32, x64, t, a, s, r, d"
			") VALUES ("
				"1.0, 2.0, 3.0, 4.0, '2017-02-27T14:47:34.123Z', 'ÖÄÜ', 'ÖÄÜ', 1.23, 3.45"
			")"
		)));
	}

	void select_special_symbols_test()
	{
		PSqlConn;

		QVERIFY(sql.exec("SELECT id, x16, x32, x64, t, a, s, r, d FROM cflib_db_test"));

		QVERIFY(sql.next());
		sql >> tt.id >> tt.x16 >> tt.x32 >> tt.x64 >> tt.t >> tt.a >> tt.s >> tt.f >> tt.d;
		QCOMPARE(tt.id,  (quint32)1);
		QCOMPARE(tt.x16, (quint16)2);
		QCOMPARE(tt.x32, (quint32)3);
		QCOMPARE(tt.x64, (quint64)4);
		QCOMPARE(tt.t, QDateTime(QDate(2017, 2, 27), QTime(14, 47, 34, 123), Qt::UTC));
		QCOMPARE(tt.a, QByteArray("ÖÄÜ"));
		QCOMPARE(tt.s, QString::fromUtf8("ÖÄÜ"));
		QVERIFY(qFuzzyCompare(tt.f, 1.23f));
		QVERIFY(qFuzzyCompare(tt.d, 3.45));
		// no futher lines
		QVERIFY(!sql.next());
		// leave table empty
		QVERIFY(sql.exec("DELETE FROM cflib_db_test WHERE id=1;"));
	}

	// -----------------------------------------------------------

	void alter_table_test()
	{
		PSqlConn;

		QVERIFY(sql.exec("ALTER TABLE cflib_db_test DROP COLUMN t;"));
		QVERIFY(sql.exec("ALTER TABLE cflib_db_test ADD COLUMN t timestamp with time zone;"));
	}

	// -----------------------------------------------------------

	void insert_time_with_time_zone_test()
	{
		PSqlConn;

		QVERIFY(sql.exec(
			"INSERT INTO "
				"cflib_db_test "
			"("
				"id, x16, x32, x64, a, s, r, d, t"
			") VALUES ("
				"1, 2, 3, 4, '漢字', 'Hello World!', 1.23, 3.45, '2017-02-27T14:47:34.123Z'"
			")"
		));
	}


	void select_time_with_time_zone_test()
	{
		PSqlConn;

		QVERIFY(sql.exec("SELECT id, x16, x32, x64, a, s, r, d, t FROM cflib_db_test"));

		QVERIFY(sql.next());
		sql >> tt.id >> tt.x16 >> tt.x32 >> tt.x64 >> tt.a >> tt.s >> tt.f >> tt.d >> tt.t;
		QCOMPARE(tt.id,  (quint32)1);
		QCOMPARE(tt.x16, (quint16)2);
		QCOMPARE(tt.x32, (quint32)3);
		QCOMPARE(tt.x64, (quint64)4);
		QCOMPARE(tt.t, QDateTime(QDate(2017, 2, 27), QTime(14, 47, 34, 123), Qt::UTC)); // or 15:47:27 without UTC
		QCOMPARE(tt.a, QByteArray("漢字"));
		QCOMPARE(tt.s, QString::fromUtf8("Hello World!"));
		QVERIFY(qFuzzyCompare(tt.f, 1.23f));
		QVERIFY(qFuzzyCompare(tt.d, 3.45));
		// no futher lines
		QVERIFY(!sql.next());
	}

	// -----------------------------------------------------------

	void update_table_test()
	{
		PSqlConn;

		QVERIFY(sql.exec(
			"UPDATE "
				"cflib_db_test "
			"SET "
				"id = 2, x16 = 123, x32 = 345, x64 = 1234567, a = '2017-02-27T14:47:34.123Z', s='Hello again', r = 123.456 , d = 12345.6789, t = '2017-02-27T14:47:34.123Z' "
			"where "
				"id = 1;"
		));
	}

	void select_updated_table_test()
	{
		PSqlConn;

		QVERIFY(sql.exec("SELECT id, x16, x32, x64, a, s, r, d, t FROM cflib_db_test"));

		QVERIFY(sql.next());
		sql >> tt.id >> tt.x16 >> tt.x32 >> tt.x64 >> tt.a >> tt.s >> tt.f >> tt.d >> tt.t;
		QCOMPARE(tt.id,  (quint32)2);
		QCOMPARE(tt.x16, (quint16)123);
		QCOMPARE(tt.x32, (quint32)345);
		QCOMPARE(tt.x64, (quint64)1234567);
		QCOMPARE(tt.t, QDateTime(QDate(2017, 2, 27), QTime(14, 47, 34, 123), Qt::UTC)); // or 15:47:27 without UTC
		QCOMPARE(tt.a, QByteArray("2017-02-27T14:47:34.123Z"));
		QCOMPARE(tt.s, QString::fromUtf8("Hello again"));
		QVERIFY(qFuzzyCompare(tt.f, 123.456f));
		QVERIFY(qFuzzyCompare(tt.d, 12345.6789));
		// no futher lines
		QVERIFY(!sql.next());
	}

	// -----------------------------------------------------------

	void select_error_test()
	{
		PSqlConn;
		QVERIFY(sql.lastFieldIsNull());

		QVERIFY(sql.exec("SELECT id, x16, x32, x64, a, s, r, d, t FROM cflib_db_test"));
		QVERIFY(sql.lastFieldIsNull());

		QVERIFY(sql.next());
		QVERIFY(sql.lastFieldIsNull());

		sql >> tt.x16;
		QVERIFY(sql.lastFieldIsNull());

		sql >> tt.t;
		QVERIFY(tt.t.isNull());
		QVERIFY(sql.lastFieldIsNull());

		QVERIFY(!sql.next());
		QVERIFY(sql.lastFieldIsNull());
	}

	// -----------------------------------------------------------

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

	// -----------------------------------------------------------

	void keepFields_test()
	{
		PSqlConn;

		sql.prepare(
			"INSERT INTO "
				"cflib_db_test "
			"("
				"id, x32"
			") VALUES ("
				"$2, $1"
			")"
		);
		sql << 123 << 5;
		QVERIFY(sql.exec(1));
		sql << 6;
		QVERIFY(sql.exec());

		sql.prepare("SELECT x32 FROM cflib_db_test WHERE id = $1");

		sql << 5;
		QVERIFY(sql.exec());
		QVERIFY(sql.next());
		sql >> tt.x32;
		QCOMPARE(tt.x32, (quint32)123);

		sql << 6;
		QVERIFY(sql.exec());
		QVERIFY(sql.next());
		sql >> tt.x32;
		QCOMPARE(tt.x32, (quint32)123);
	}

	// -----------------------------------------------------------

	void multi_prepare_test()
	{
		PSqlConn;

		sql.prepare(
			"INSERT INTO "
				"cflib_db_test "
			"("
				"id"
			") VALUES ("
				"$1"
			")"
		);

		PSql sql2;

		sql2.prepare("SELECT id FROM cflib_db_test WHERE id = $1");

		sql << 7;
		sql2 << 6;
		QVERIFY(sql.exec());
		QVERIFY(sql2.exec());
		QVERIFY(sql2.next());
		sql2 >> tt.id;
		QCOMPARE(tt.id, (quint32)6);
		QVERIFY(!sql2.next());

		sql << 8;
		sql2 << 7;
		QVERIFY(sql.exec());
		QVERIFY(sql2.exec());
		QVERIFY(sql2.next());
		sql2 >> tt.id;
		QCOMPARE(tt.id, (quint32)7);
		QVERIFY(!sql2.next());
	}

	void cascading_multi_prepare_test()
	{
		PSqlConn;

		QVERIFY(sql.exec("SELECT COUNT(*) FROM cflib_db_test"));
		QVERIFY(sql.next());
		sql >> tt.x64;
		QVERIFY(tt.x64 > (quint32)1);

		sql.prepare("SELECT id FROM cflib_db_test ORDER BY id");
		QVERIFY(sql.exec());

		PSql sql2;
		sql2.prepare("SELECT x32 FROM cflib_db_test WHERE id = $1");

		QVERIFY(sql.next());
		sql >> tt.id;
		QCOMPARE(tt.id, (quint32)2);

		sql2 << tt.id;
		QVERIFY(sql2.exec());
		QVERIFY(sql2.next());
		sql2 >> tt.x32;
		QCOMPARE(tt.x32, (quint32)345);
		QVERIFY(!sql2.next());

		// cascading queries do not work
		QVERIFY(!sql.next());
	}

	void two_connections_test()
	{
		PSqlConn;

		QVERIFY(sql.exec("SELECT COUNT(*) FROM cflib_db_test"));
		QVERIFY(sql.next());
		sql >> tt.x64;
		QVERIFY(tt.x64 > (quint32)2);

		sql.prepare("SELECT id FROM cflib_db_test ORDER BY id");
		QVERIFY(sql.exec());

		PSql sql2("");
		sql2.prepare("SELECT x32 FROM cflib_db_test WHERE id = $1");

		QVERIFY(sql.next());
		sql >> tt.id;
		QCOMPARE(tt.id, (quint32)2);

		sql2 << tt.id;
		QVERIFY(sql2.exec());
		QVERIFY(sql2.next());
		sql2 >> tt.x32;
		QCOMPARE(tt.x32, (quint32)345);
		QVERIFY(!sql2.next());

		QVERIFY(sql.next());
		sql >> tt.id;
		QCOMPARE(tt.id, (quint32)5);

		sql2 << tt.id;
		QVERIFY(sql2.exec());
		QVERIFY(sql2.next());
		sql2 >> tt.x32;
		QCOMPARE(tt.x32, (quint32)123);
		QVERIFY(!sql2.next());

		QVERIFY(sql.next());
	}

};
#include "psql_test.moc"
ADD_TEST(PSql_test)

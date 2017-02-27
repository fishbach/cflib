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
				"id integer NOT NULL,"
				"x16 smallint,"
				"x32 integer,"
				"x64 bigint,"
				"t varchar(255),"
				"PRIMARY KEY (id)"
			")"));
	}

	void cleanupTestCase()
	{
		PSqlConn;
		QVERIFY(sql.exec("DROP TABLE cflib_db_test"));
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
				"id, x16, x32, x64, t"
			") VALUES ("
				"1, 2, 3, 4, 't'"
			")"));
		QVERIFY(!sql.next());

		QVERIFY(sql.exec(
			"INSERT INTO "
				"cflib_db_test "
			"("
				"id, x16, x32, x64, t"
			") VALUES ("
				"2, 5, 6, 7, 't'"
			")"));

		QVERIFY( sql.exec("SELECT id, x16, x32, x64, t FROM cflib_db_test"));

		quint32 id;
		quint16 x16;
		quint32 x32;
		quint64 x64;

		QVERIFY(sql.next());
		sql >> id >> x16 >> x32 >> x64;
		QCOMPARE(id,  (quint32)1);
		QCOMPARE(x16, (quint16)2);
		QCOMPARE(x32, (quint32)3);
		QCOMPARE(x64, (quint64)4);

		QVERIFY(sql.next());
		sql >> id >> x16 >> x32 >> x64;
		QCOMPARE(id,  (quint32)2);
		QCOMPARE(x16, (quint16)5);
		QCOMPARE(x32, (quint32)6);
		QCOMPARE(x64, (quint64)7);

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

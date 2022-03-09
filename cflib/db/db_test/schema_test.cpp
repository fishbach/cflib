/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/db/psql.h>
#include <cflib/db/schema.h>
#include <cflib/util/log.h>
#include <cflib/util/test.h>
#include <cflib/util/util.h>

using namespace cflib::db;
using namespace cflib::util;

USE_LOG(LogCat::Db)

namespace {

class Migrator : public QObject
{
	Q_OBJECT
public slots:
	bool test1()
	{
		PSqlConn;
		return sql.exec("INSERT INTO config (key, value) VALUES ('test1', 'val1')");
	}

	bool test2()
	{
		PSqlConn;
		return sql.exec("INSERT INTO config (key, value, value2) VALUES ('test2', 'val2', 2)");
	}

	bool test3()
	{
		PSqlConn;
		return sql.exec("INSERT INTO config (key, value, value2) VALUES ('test3', 'val3', 3)");
	}

	bool test4()
	{
		PSqlConn;
		return sql.exec("INSERT INTO config (key, value, value2, value3) VALUES ('test4', 'val4', 4, 4)");
	}

	bool test5()
	{
		PSqlConn;
		return sql.exec("INSERT INTO config (key, value, value2, value3, value4) VALUES ('test5', 'val5', 5, 5, 5)");
	}
};

}

class Schema_test : public QObject
{
	Q_OBJECT
private slots:

	void initTestCase()
	{
		PSql sql("host=127.0.0.1 dbname=postgres");
		sql.exec("DROP DATABASE cflib_db_test");
		QVERIFY(sql.exec("CREATE DATABASE cflib_db_test"));
		QVERIFY(PSql::setParameter("host=127.0.0.1 dbname=cflib_db_test"));
	}

	void cleanupTestCase()
	{
		PSql::closeConnection();
		PSql sql("host=127.0.0.1 dbname=postgres");
		QVERIFY(sql.exec("DROP DATABASE cflib_db_test"));
	}

	void basic_test()
	{
		QVERIFY(schema::update<Migrator>());

		PSqlConn;
		QVERIFY(sql.exec("SELECT key, value, value2, value3, value4 FROM config ORDER BY key"));
		QString key, value;
		qint32 value2, value3, value4;

		QVERIFY(sql.next());
		sql >> key >> value >> value2 >> value3 >> value4;
		QCOMPARE(key, QString("test1"));
		QCOMPARE(value, QString("val1"));
		QCOMPARE(value2, 0);
		QCOMPARE(value3, 0);
		QCOMPARE(value4, 0);

		QVERIFY(sql.next());
		sql >> key >> value >> value2 >> value3 >> value4;
		QCOMPARE(key, QString("test2"));
		QCOMPARE(value, QString("val2"));
		QCOMPARE(value2, 2);
		QCOMPARE(value3, 0);
		QCOMPARE(value4, 0);

		QVERIFY(sql.next());
		QVERIFY(sql.next());
		QVERIFY(sql.next());
		QVERIFY(!sql.next());
	}

	void update_test()
	{
		QByteArray schema = readFile(":/schema.sql");
		QVERIFY(schema::update(schema));

		schema +=
			"-- REVISION neu\n"
			"\n"
			"INSERT INTO config (key) VALUES ('neu')\n"
		;
		QVERIFY(schema::update(schema));
		PSqlConn;
		QVERIFY(sql.exec("SELECT COUNT(*) FROM config WHERE key = 'neu'"));
		QVERIFY(sql.next());
		QCOMPARE(sql.get<qint64>(0), 1);
	}

	void resetDB()
	{
		PSql::closeConnection();
		PSql sql("host=127.0.0.1 dbname=postgres");
		QVERIFY(sql.exec("DROP DATABASE cflib_db_test"));
		QVERIFY(sql.exec("CREATE DATABASE cflib_db_test"));
		QVERIFY(PSql::setParameter("host=127.0.0.1 dbname=cflib_db_test"));
	}

	void empty_head_test()
	{
		QVERIFY(schema::update(
			"-- REVISION first\n"
			"CREATE TABLE config (\n"
			"  key   text NOT NULL, \n"
			"  value text, \n"
			"  PRIMARY KEY (key)\n"
			");\n"
		));

		PSqlConn;
		QVERIFY(sql.exec("SELECT key, value FROM config"));
	}

};
#include "schema_test.moc"
ADD_TEST(Schema_test)

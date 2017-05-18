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

#include <cflib/db/psql.h>
#include <cflib/db/schema.h>
#include <cflib/util/test.h>
#include <cflib/util/log.h>

using namespace cflib::db;

USE_LOG(LogCat::Db)

namespace {

class Migrator : public QObject
{
	Q_OBJECT
public slots:
	bool test1()
	{
		QTextStream(stdout) << "test1 !!!!!!!!!!!\n";
		return true;
	}

	bool test2()
	{
		QTextStream(stdout) << "test2 !!!!!!!!!!!\n";
		return true;
	}

	bool test3()
	{
		QTextStream(stdout) << "test3 !!!!!!!!!!!\n";
		return true;
	}

	bool test4()
	{
		QTextStream(stdout) << "test4 !!!!!!!!!!!\n";
		return true;
	}

	bool test5()
	{
		QTextStream(stdout) << "test5 !!!!!!!!!!!\n";
		return true;
	}
};

}

class Schema_test : public QObject
{
	Q_OBJECT
private slots:

	void initTestCase()
	{
		{
			PSql sql("host=127.0.0.1 dbname=postgres");
			sql.exec("DROP DATABASE cflib_db_test");
			QVERIFY(sql.exec("CREATE DATABASE cflib_db_test"));
		}
		QVERIFY(PSql::setParameter("host=127.0.0.1 dbname=cflib_db_test"));
	}

	void cleanupTestCase()
	{
		PSqlConn;
//		QVERIFY(sql.exec("DROP DATABASE cflib_db_test"));
	}

	void basic_test()
	{
		QVERIFY(updateSchema<Migrator>());
	}

};
#include "schema_test.moc"
ADD_TEST(Schema_test)

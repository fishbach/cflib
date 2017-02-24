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
	}

	void some_test()
	{
		PSqlConn;
		QVERIFY( sql.exec("SELECT 42"));
		QVERIFY(!sql.exec("SULICT 42"));
		QVERIFY(sql.commit());
	}

};
#include "psql_test.moc"
ADD_TEST(PSql_test)

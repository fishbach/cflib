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

#include "psql.h"

#include <cflib/util/log.h>

#include <libpq-fe.h>

USE_LOG(LogCat::Db)

namespace cflib { namespace db {

namespace {

QString connInfo;

}

bool PSql::setParameter(const QString & connectionParameter)
{
	connInfo = QString();

	PGconn * conn = PQconnectdb(connectionParameter.toUtf8().constData());
	if (PQstatus(conn) != CONNECTION_OK) {
		logWarn("cannot connect to database (error: %1)", PQerrorMessage(conn));
		PQfinish(conn);
		return false;
	}

	PGresult * res = PQexec(conn, "SELECT 'char'::regtype::oid, 'bigint'::regtype::oid");
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		logWarn("cannot get oids (error: %1)", PQerrorMessage(conn));
		PQclear(res);
		PQfinish(conn);
		return false;
	}

	for (int i = 0 ; i < PQntuples(res) ; i++) {
		QTextStream(stdout) << "r: " << PQgetvalue(res, i, 0) << ", " << PQgetvalue(res, i, 1) << endl;
	}

	PQclear(res);

	PQfinish(conn);

	connInfo = connectionParameter;
	return true;
}

}}	// namespace

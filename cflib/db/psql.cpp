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

enum PostgresTypes {
	PSql_int16 = 1,
	PSql_int32,
	PSql_int64,
	PSql_float,
	PSql_double,
	PSql_string,
	PSql_binary,
	PSql_timestamp
};

QHash<PostgresTypes, Oid> typeOids;

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

	PGresult * res = PQexec(conn,
		"SELECT "
			"'smallint'::regtype::oid, "
			"'integer'::regtype::oid, "
			"'bigint'::regtype::oid, "
			"'real'::regtype::oid,"
			"'double precision'::regtype::oid,"
			"'text'::regtype::oid, "
			"'bytea'::regtype::oid, "
			"'timestamp with time zone'::regtype::oid"
	);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		logWarn("cannot get oids (error: %1)", PQerrorMessage(conn));
		PQclear(res);
		PQfinish(conn);
		return false;
	}

	if (PQntuples(res) != 1) {
		logWarn("funny result count");
		PQclear(res);
		PQfinish(conn);
		return false;
	}

	typeOids[PSql_int16    ] = (Oid)QByteArray(PQgetvalue(res, 0, 0)).toUInt();
	typeOids[PSql_int32    ] = (Oid)QByteArray(PQgetvalue(res, 0, 1)).toUInt();
	typeOids[PSql_int64    ] = (Oid)QByteArray(PQgetvalue(res, 0, 2)).toUInt();
	typeOids[PSql_float    ] = (Oid)QByteArray(PQgetvalue(res, 0, 3)).toUInt();
	typeOids[PSql_double   ] = (Oid)QByteArray(PQgetvalue(res, 0, 4)).toUInt();
	typeOids[PSql_string   ] = (Oid)QByteArray(PQgetvalue(res, 0, 5)).toUInt();
	typeOids[PSql_binary   ] = (Oid)QByteArray(PQgetvalue(res, 0, 6)).toUInt();
	typeOids[PSql_timestamp] = (Oid)QByteArray(PQgetvalue(res, 0, 7)).toUInt();

	PQclear(res);

	PQfinish(conn);

	connInfo = connectionParameter;
	return true;
}

}}	// namespace

/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
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

#include "dbconfigpsql.h"

#include <cflib/db/psql.h>

USE_LOG(LogCat::Db)

namespace cflib { namespace db {

QMap<QString, QString> getConfigPSql()
{
	QMap<QString, QString> retval;

	PSqlConn;
	sql.begin();
	if (!sql.exec(
		"SELECT "
			"key, value "
		"FROM "
			"config"
		)) return retval;
	while (sql.next()) {
		retval[sql.get<QString>(0)] = sql.get<QString>(1);
	}
	sql.commit();

	return retval;
}

cflib::util::Mail getMailTemplatePSql(const QString & name, const QString & lang)
{
	cflib::util::Mail retval;

	PSqlConn;
	sql.begin();
	sql.prepare(
		"SELECT "
			"subject, text "
		"FROM "
			"mailTemplates "
		"WHERE "
			"name = $1 AND lang = $2"
	);
	sql << name << lang;
	if (!sql.exec()) return retval;
	if (sql.next()) {
		sql >> retval.subject >> retval.text;
	}

	return retval;
}

}}	// namespace

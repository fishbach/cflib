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

#include "dbconfig.h"

#include <cflib/db/db.h>

USE_LOG(LogCat::Db)

namespace cflib { namespace db {

QMap<QString, QString> getConfig()
{
	QMap<QString, QString> retval;

	Transaction;
	QSqlQuery query(ta.db);
	query.prepare(
		"SELECT "
			"`key`, `value` "
		"FROM "
			"`config`"
	);
	if (!execQuery(query)) return retval;

	while (query.next()) {
		retval[query.value(0).toString()] = query.value(1).toString();
	}

	ta.commit();
	return retval;
}

cflib::util::Mail getMailTemplate(const QString & name, const QString & lang)
{
	cflib::util::Mail retval;

	Transaction;
	QSqlQuery query(ta.db);
	query.prepare(
		"SELECT "
			"`subject`, `text` "
		"FROM "
			"mailTemplates "
		"WHERE "
			"name = :name AND lang = :lang"
	);
	query.bindValue(":name", name);
	query.bindValue(":lang", lang);
	if (!execQuery(query)) return retval;

	if (query.next()) {
		retval.subject = query.value(0).toString();
		retval.text    = query.value(1).toString();
	}

	ta.commit();
	return retval;
}

}}	// namespace

/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
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

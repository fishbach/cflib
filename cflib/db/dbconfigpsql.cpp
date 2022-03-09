/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
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

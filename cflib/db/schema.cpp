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

#include "schema.h"

#include <cflib/db/psql.h>
#include <cflib/util/log.h>
#include <cflib/util/util.h>

USE_LOG(LogCat::Db)

namespace cflib { namespace db {

namespace {

bool insertRevision(const QString & rev)
{
	PSqlConn;
	sql.prepare(
		"INSERT INTO "
			"__scheme_revisions__ "
		"("
			"rev, applied"
		") VALUES ("
			"?, ?"
		")");
	sql << rev << QDateTime::currentDateTimeUtc();
	return sql.exec();
}

}

bool updateSchema(const QString & filename, QObject * migrator)
{
	return updateSchema(util::readFile(filename), migrator);
}

bool updateSchema(const QByteArray & schema, QObject * migrator)
{
	PSqlConn;

	// get existing revisions
	QSet<QString> existingRevisions;
	bool isInitial = false;
	if (!sql.exec("SELECT rev FROM __scheme_revisions__")) {
		if (!sql.exec(
			"CREATE TABLE __scheme_revisions__ ("
				"rev text NOT NULL, "
				"applied timestamp with time zone NOT NULL"
			")"))
		{
			return false;
		}
		isInitial = true;
	} else {
		while (sql.next()) {
			existingRevisions << sql.get<QString>(0);
		}
	}

	const QRegularExpression revRe("(?:^|\n)-- REVISION (.+)(?:\n|$)");

	const QString utf8Schema = QString::fromUtf8(schema);

	int start = 0;
	QRegularExpressionMatch match = revRe.match(utf8Schema, start);
	QString lastRev;
	while (match.hasMatch()) {
		if (lastRev.isNull()) lastRev = "__initial__";
		if (!existingRevisions.contains(lastRev)) {
			QTextStream(stdout) << "==>" << utf8Schema.mid(start, match.capturedStart()) << "<==" << endl;
			if (!insertRevision(lastRev)) return false;
		}
		lastRev = match.captured(1);
		start = match.capturedEnd();
		match = revRe.match(utf8Schema, start);
	}
	if (lastRev.isNull()) lastRev = "__initial__";
	if (!existingRevisions.contains(lastRev)) {
		QTextStream(stdout) << "===\n" << utf8Schema.mid(start, match.capturedStart()) << endl;
		if (!insertRevision(lastRev)) return false;
	}

	return true;
}

}}	// namespace

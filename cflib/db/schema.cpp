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

namespace cflib { namespace db { namespace schema {

namespace {

class MigratorObject : public QObject
{
public:
	MigratorObject(Migrator migrator) : migrator_(migrator) {}
	bool migrate(const QByteArray & name) { return migrator_(name); }
private:
	const Migrator migrator_;
};

bool insertRevision(const QString & rev)
{
	PSqlConn;
	sql.prepare(
		"INSERT INTO "
			"__scheme_revisions__ "
		"("
			"rev, applied"
		") VALUES ("
			"$1, $2"
		") ON CONFLICT DO NOTHING"
	);
	sql << rev << QDateTime::currentDateTimeUtc();
	return sql.exec();
}

bool confirmRevision(const QString & rev)
{
	PSqlConn;
	sql.prepare(
		"UPDATE "
			"__scheme_revisions__ "
		"SET "
			"applied = $1, success = 1 "
		"WHERE "
			"rev = $2"
	);
	sql << QDateTime::currentDateTimeUtc() << rev;
	return sql.exec();
}

bool execSql(const QString & query)
{
	static const QRegularExpression commentRe("(?:^|\n)--.+");

	QString cleanQuery = query;
	cleanQuery.replace(commentRe, "");
	cleanQuery = cleanQuery.trimmed();
	if (cleanQuery.isEmpty()) return true;

	PSqlConn;
	logDebug("executing: %1", cleanQuery);
	return sql.execMultiple(cleanQuery);
}

bool execRevision(const QString & query, QObject * migrator)
{
	static const QRegularExpression execRe("^-- EXEC (.+)$", QRegularExpression::MultilineOption);

	int start = 0;
	QRegularExpressionMatch match = execRe.match(query, start);
	while (match.hasMatch()) {
		QByteArray method = match.captured(1).toUtf8().trimmed();

		if (!execSql(query.mid(start, match.capturedStart() - start))) return false;

		if (!migrator) {
			logWarn("found EXEC in SQL, but no migrator given");
			return false;
		}

		MigratorObject * migratorObject = dynamic_cast<MigratorObject *>(migrator);
		if (migratorObject) {
			if (!migratorObject->migrate(method)) {
				logWarn("migration %1 failed", method);
				return false;
			}
		} else {
			const QMetaObject * meta = migrator->metaObject();
			if (!meta) {
				logWarn("migrator has no meta Object");
				return false;
			}
			int methodId = meta->indexOfMethod(method + "()");
			if (methodId == -1) {
				logWarn("method void %1 not found in migrator", method);
				return false;
			}
			bool ok = false;
			if (!meta->method(methodId).invoke(migrator, Q_RETURN_ARG(bool, ok))) {
				logWarn("could not invoke method void %1 of migrator", method);
				return false;
			}
			if (!ok) {
				logWarn("migration method %1 failed", method);
				return false;
			}
		}

		logInfo("migration %1 finished successfully", method);

		start = match.capturedEnd();
		match = execRe.match(query, start);
	}
	return execSql(query.mid(start));
}

}

bool update(QObject * migrator, const QString & filename)
{
	return update(util::readFile(filename), migrator);
}

bool update(Migrator migrator, const QString & filename)
{
	return update(util::readFile(filename), migrator);
}

bool update(const QByteArray & schema, QObject * migrator)
{
	PSqlConn;

	// get existing revisions
	QSet<QString> existingRevisions;
	if (!sql.exec("SELECT rev FROM __scheme_revisions__ WHERE success = 1")) {
		logInfo("creating table __scheme_revisions__");
		if (!sql.exec(
			"CREATE TABLE __scheme_revisions__ ("
				"rev text NOT NULL, "
				"applied timestamp with time zone NOT NULL, "
				"success smallint NOT NULL DEFAULT 0, "
				"PRIMARY KEY (rev)"
			")"
		)) return false;
	} else {
		while (sql.next()) {
			existingRevisions << sql.get<QString>(0);
		}
	}

	const QRegularExpression revRe("^-- REVISION (.+)$", QRegularExpression::MultilineOption);

	const QString utf8Schema = QString::fromUtf8(schema);

	int start = 0;
	QRegularExpressionMatch match = revRe.match(utf8Schema, start);
	QString lastRev = "__initial__";
	while (match.hasMatch()) {
		if (!existingRevisions.contains(lastRev)) {
			logInfo("applying revision %1", lastRev);
			if (!insertRevision(lastRev)) return false;
			PSqlConn;
			sql.begin();
			if (!execRevision(utf8Schema.mid(start, match.capturedStart() - start), migrator)) return false;
			if (!confirmRevision(lastRev)) return false;
			if (!sql.commit()) return false;
		}

		lastRev = match.captured(1);
		start = match.capturedEnd();
		match = revRe.match(utf8Schema, start);
	}

	if (!existingRevisions.contains(lastRev)) {
		logInfo("applying revision %1", lastRev);
		if (!insertRevision(lastRev)) return false;
		PSqlConn;
		sql.begin();
		if (!execRevision(utf8Schema.mid(start), migrator)) return false;
		if (!confirmRevision(lastRev)) return false;
		if (!sql.commit()) return false;
	}

	return true;
}

bool update(const QByteArray & schema, Migrator migrator)
{
	if (!migrator) return update(schema);
	MigratorObject migratorObject(migrator);
	return update(schema, &migratorObject);
}

}}}	// namespace

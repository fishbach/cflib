/* Copyright (C) 2013-2014 Christian Fischbach <cf@cflib.de>
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

#include "db.h"

#include <cflib/util/log.h>

USE_LOG(LogCat::Db)

namespace cflib { namespace db {

namespace {

QString dbDatabase;
QString dbUser;
QString dbPassword;
QAtomicInt dbNameId;

QSqlDatabase createNewConnection()
{
	return QSqlDatabase::addDatabase("QMYSQL", "DB-" + QString::number(dbNameId.fetchAndAddRelaxed(1)));
}

class ThreadData : public QObject
{
	Q_OBJECT
public:
	QSqlDatabase * db;
	bool transactionActive;
	bool doRollback;

	ThreadData() :
		db(new QSqlDatabase(createNewConnection())),
		transactionActive(false),
		doRollback(false)
	{
		logDebug("new DB connection: %1", db->connectionName());
		db->setHostName("127.0.0.1");
		db->setPort(3306);
		db->setDatabaseName(dbDatabase);
		db->setUserName(dbUser);
		db->setPassword(dbPassword);
		if (!db->open()) logWarn("DB error: %1", db->lastError().text());

		QTimer * timer = new QTimer(this);
		connect(timer, SIGNAL(timeout()), this, SLOT(checkConnection()));
		timer->start(60 * 1000);
	}
	~ThreadData()
	{
		QString name = db->connectionName();
		db->close();
		delete db;
		QSqlDatabase::removeDatabase(name);
		logDebug("DB connection %1 closed", name);
	}

private slots:
	void checkConnection()
	{
		QTime watch;
		watch.start();
		if (!QSqlQuery(*db).exec("SELECT 1")) logWarn("DB error on connection check: %1", db->lastError().text());
		int elapsed = watch.elapsed();
		if (elapsed >= 5) logTrace("DB connection responsiveness: %1 msec", elapsed);
	}
};
QThreadStorage<ThreadData *> threadData;

inline ThreadData & getThreadData()
{
	if (!threadData.hasLocalData()) threadData.setLocalData(new ThreadData());
	return *(threadData.localData());
}

#include "db.moc"
}

ScopedTransaction::ScopedTransaction(const util::LogFileInfo & lfi, int line) :
	db(*getThreadData().db),
	lfi_(lfi), line_(line),
	committed_(false)
{
	ThreadData & td = getThreadData();
	nested_ = td.transactionActive;
	cflib::util::Log(lfi_, line_, LogCat::Debug | LogCat::Db)("DB %1transaction start", nested_ ? "sub-" : "");
	if (!nested_) {
		td.transactionActive = true;
		if (!db.transaction()) logCritical("starting DB transaction failed: %1", db.lastError().text());
		watch_.start();
	}
}

ScopedTransaction::~ScopedTransaction()
{
	ThreadData & td = getThreadData();
	if (!committed_) td.doRollback = true;
	if (td.doRollback) {
		cflib::util::Log(lfi_, line_, LogCat::Info | LogCat::Db)("DB %1tansaction rollback", nested_ ? "sub-" : "");
		if (!nested_) {
			if (!db.rollback()) logWarn("DB transaction rollback failed: %1", db.lastError().text());
			td.doRollback = false;
			td.transactionActive = false;
		}
	}
}

bool ScopedTransaction::commit()
{
	if (committed_) {
		logCritical("DB transaction committed twice");
		return false;
	}
	committed_ = true;

	ThreadData & td = getThreadData();

	if (td.doRollback) return false;

	if (nested_) {
		cflib::util::Log(lfi_, line_, LogCat::Debug | LogCat::Db)("DB sub-transaction commit");
		return true;
	}

	QTime watch;
	watch.start();
	bool ok = db.commit();
	if (!ok) {
		cflib::util::Log(lfi_, line_, LogCat::Warn | LogCat::Db)(
			"DB transaction commit failed: %1", db.lastError().text());
	} else {
		cflib::util::Log(lfi_, line_, LogCat::Debug | LogCat::Db)(
			"DB transaction commit (%1/%2 msec)", watch.elapsed(), watch_.elapsed());
	}

	td.transactionActive = false;
	return ok;
}

void setParameter(const QString & database, const QString & user, const QString & password)
{
	dbDatabase = database;
	dbUser = user;
	dbPassword = password;
}

bool exec(QSqlQuery & query, const cflib::util::LogFileInfo & lfi, int line)
{
	if (!query.lastError().isValid()) {
		if (query.exec()) return true;
	}
	cflib::util::Log(lfi, line, LogCat::Debug | LogCat::Db)("Query: %1", query.lastQuery());
	cflib::util::Log(lfi, line, LogCat::Warn  | LogCat::Db)("SQL error: %1", query.lastError().text());
	return false;
}

void closeDBConnection()
{
	threadData.setLocalData(0);
}

QDateTime getUTC(const QSqlQuery & query, int index)
{
	QDateTime retval = query.value(index).toDateTime();
	retval.setTimeSpec(Qt::UTC);
	return retval;
}

quint32 lastInsertId()
{
	Transaction;
	QSqlQuery query(ta.db);
	query.prepare("SELECT LAST_INSERT_ID()");
	if (!execQuery(query) || !query.next()) return 0;
	quint32 rv = query.value(0).toUInt();
	ta.commit();
	return rv;
}

}}	// namespace

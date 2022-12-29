/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "db.h"

#include <cflib/util/evtimer.h>
#include <cflib/util/threadverify.h>

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

}

class ScopedTransaction::ThreadData : public QObject
{
	Q_OBJECT
public:
	QSqlDatabase * db;
	bool transactionActive;
	bool doRollback;
	util::EVTimer * evTimer_;

	ThreadData() :
		db(new QSqlDatabase(createNewConnection())),
		transactionActive(false),
		doRollback(false),
		evTimer_(0)
	{
		logDebug("new DB connection: %1", db->connectionName());
		db->setHostName("127.0.0.1");
		db->setPort(3306);
		db->setDatabaseName(dbDatabase);
		db->setUserName(dbUser);
		db->setPassword(dbPassword);
		if (!db->open()) logWarn("DB error: %1", db->lastError().text());


		if (util::libEVLoopOfThread()) {
			evTimer_ = new util::EVTimer(this, &ThreadData::checkConnection);
			evTimer_->start(15);
		} else {
			QTimer * timer = new QTimer(this);
			connect(timer, SIGNAL(timeout()), this, SLOT(checkConnection()));
			timer->start(15 * 1000);
		}
	}
	~ThreadData()
	{
		delete evTimer_;
		QString name = db->connectionName();
		db->close();
		delete db;
		QSqlDatabase::removeDatabase(name);
		logDebug("DB connection %1 closed", name);
	}

private slots:
	void checkConnection()
	{
		QElapsedTimer watch;
		watch.start();
		QSqlQuery query(*db);
		if (!query.exec("SELECT 1") || !query.next()) {
			logWarn("DB error on connection check: %1", db->lastError().text());
			if (!db->open()) logWarn("DB error: %1", db->lastError().text());
		} else {
			const int elapsed = watch.elapsed();
			if (elapsed >= 5) logTrace("DB connection responsiveness: %1 msec", elapsed);
		}
	}
};

QThreadStorage<ScopedTransaction::ThreadData *> ScopedTransaction::threadData_;

ScopedTransaction::ScopedTransaction(const util::LogFileInfo & lfi, int line) :
	td_(threadData_.hasLocalData() ? *(threadData_.localData()) : (
		threadData_.setLocalData(new ThreadData()), *(threadData_.localData()))),
	db(*td_.db),
	lfi_(lfi), line_(line),
	committed_(false)
{
	nested_ = td_.transactionActive;
	cflib::util::Log(lfi_, line_, LogCat::Debug | LogCat::Db)("DB %1transaction start", nested_ ? "sub-" : "");
	if (!nested_) {
		td_.transactionActive = true;
		if (!db.transaction()) logCritical("starting DB transaction failed: %1", db.lastError().text());
		watch_.start();
	}
}

ScopedTransaction::~ScopedTransaction()
{
	if (!committed_) td_.doRollback = true;
	if (td_.doRollback) {
		cflib::util::Log(lfi_, line_, LogCat::Info | LogCat::Db)("DB %1tansaction rollback", nested_ ? "sub-" : "");
		if (!nested_) {
			if (!db.rollback()) logWarn("DB transaction rollback failed: %1", db.lastError().text());
			td_.doRollback = false;
			td_.transactionActive = false;
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

	if (td_.doRollback) return false;

	if (nested_) {
		cflib::util::Log(lfi_, line_, LogCat::Debug | LogCat::Db)("DB sub-transaction commit");
		return true;
	}

	QElapsedTimer watch;
	watch.start();
	bool ok = db.commit();
	if (!ok) {
		cflib::util::Log(lfi_, line_, LogCat::Warn | LogCat::Db)(
			"DB transaction commit failed: %1", db.lastError().text());
	} else {
		cflib::util::Log(lfi_, line_, LogCat::Debug | LogCat::Db)(
			"DB transaction commit (%1/%2 msec)", watch.elapsed(), watch_.elapsed());
	}

	td_.transactionActive = false;
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
	ScopedTransaction::threadData_.setLocalData(0);
}

QDateTime getUTC(const QSqlQuery & query, int index)
{
	QDateTime retval = query.value(index).toDateTime();
	retval.setTimeSpec(Qt::UTC);
	return retval;
}

quint64 lastInsertId()
{
	Transaction;
	QSqlQuery query(ta.db);
	query.prepare("SELECT LAST_INSERT_ID()");
	if (!execQuery(query) || !query.next()) return 0;
	quint64 rv = query.value(0).toULongLong();
	ta.commit();
	return rv;
}

}}	// namespace

#include "db.moc"

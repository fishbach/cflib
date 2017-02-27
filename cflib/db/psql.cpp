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

#include <cflib/util/evtimer.h>
#include <cflib/util/threadverify.h>

#include <libpq-fe.h>

USE_LOG(LogCat::Db)

namespace cflib { namespace db {

namespace {

QAtomicInt connIdCounter(1);

enum PostgresTypes {
	PSql_int16 = 1,
	PSql_int32,
	PSql_int64,
	PSql_float,
	PSql_double,
	PSql_string,
	PSql_binary,
	PSql_timestamp,
	PSql_lastEntry = PSql_timestamp
};

Oid typeOids[PSql_lastEntry + 1];
QString connInfo;

class ThreadData : public QObject
{
	Q_OBJECT
public:
	const int connId;
	PGconn * conn;
	bool transactionActive;
	bool doRollback;
	util::EVTimer * evTimer_;

public:
	ThreadData() :
		connId(connIdCounter.fetchAndAddRelaxed(1)),
		transactionActive(false),
		doRollback(false),
		evTimer_(0)
	{
		logDebug("new DB connection: %1", connId);

		conn = PQconnectdb(connInfo.toUtf8().constData());
		if (PQstatus(conn) != CONNECTION_OK) {
			logWarn("cannot connect to database (error: %1)", PQerrorMessage(conn));
			PQfinish(conn);
			return;
		}

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
		PQfinish(conn);
		logDebug("DB connection %1 closed", connId);
	}

private slots:
	void checkConnection()
	{
		QElapsedTimer watch;
		watch.start();

		PGresult * res = PQexec(conn, "SELECT 1");
		if (PQresultStatus(res) != PGRES_TUPLES_OK) {
			logWarn("DB error on connection %1 check: %2", connId, PQerrorMessage(conn));
			PQclear(res);
			PQfinish(conn);

			// try reconnect
			conn = PQconnectdb(connInfo.toUtf8().constData());
			if (PQstatus(conn) != CONNECTION_OK) {
				logWarn("cannot connect to database (error: %1)", PQerrorMessage(conn));
				PQfinish(conn);
			}
		} else {
			int elapsed = watch.elapsed();
			if (elapsed >= 5) logTrace("DB connection %1 responsiveness: %2 msec", connId, elapsed);
			PQclear(res);
		}
	}

};
QThreadStorage<ThreadData *> threadData;

inline ThreadData & getThreadData()
{
	if (!threadData.hasLocalData()) threadData.setLocalData(new ThreadData());
	return *(threadData.localData());
}

}

const int PSql::MAX_FIELD_COUNT;

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

void PSql::closeConnection()
{
	threadData.setLocalData(0);
}

PSql::PSql(const util::LogFileInfo & lfi, int line) :
	td_(getThreadData()),
	lfi_(lfi), line_(line),
	nestedTransaction_(td_.transactionActive),
	localTransactionActive_(false),
	isFirstResult_(true),
	res_(0),
	haveResultInfo_(false),
	resultFieldCount_(-1),
	resultFieldTypes_{},
	currentFieldId_(-1)

{
}

PSql::~PSql()
{
	clearResult();
	if (localTransactionActive_) rollback();
}

void PSql::begin()
{
	if (localTransactionActive_) {
		cflib::util::Log(lfi_, line_, LogCat::Warn | LogCat::Db)(
			"begin called with active transaction");
		return;
	}
	localTransactionActive_ = true;

	if (nestedTransaction_) {
		cflib::util::Log(lfi_, line_, LogCat::Debug | LogCat::Db)("DB sub-transaction start");
		return;
	}

	cflib::util::Log(lfi_, line_, LogCat::Debug | LogCat::Db)("DB transaction start");
	td_.transactionActive = true;
	watch_.start();

	PGresult * res = PQexec(td_.conn, "BEGIN");
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		cflib::util::Log(lfi_, line_, LogCat::Critical | LogCat::Db)(
			"starting DB transaction failed: %1", PQerrorMessage(td_.conn));
	}
	PQclear(res);
}

bool PSql::commit()
{
	if (td_.doRollback) {
		rollback();
		return false;
	}

	if (!localTransactionActive_) {
		cflib::util::Log(lfi_, line_, LogCat::Warn | LogCat::Db)(
			"commit called without active transaction");
		return false;
	}
	localTransactionActive_ = false;

	if (nestedTransaction_) {
		cflib::util::Log(lfi_, line_, LogCat::Debug | LogCat::Db)("DB sub-transaction commit");
		return true;
	}

	bool ok;
	QElapsedTimer watch;
	watch.start();
	PGresult * res = PQexec(td_.conn, "COMMIT");
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		ok = false;
		cflib::util::Log(lfi_, line_, LogCat::Warn | LogCat::Db)(
			"DB transaction commit failed: %1", PQerrorMessage(td_.conn));
	} else {
		ok = true;
		cflib::util::Log(lfi_, line_, LogCat::Debug | LogCat::Db)(
			"DB transaction commit (%1/%2 msec)", watch.elapsed(), watch_.elapsed());
	}
	PQclear(res);

	td_.transactionActive = false;
	return ok;
}

void PSql::rollback()
{
	if (!localTransactionActive_) {
		cflib::util::Log(lfi_, line_, LogCat::Warn | LogCat::Db)(
			"rollback called without active transaction");
		return;
	}
	localTransactionActive_ = false;

	if (nestedTransaction_) {
		cflib::util::Log(lfi_, line_, LogCat::Info | LogCat::Db)("DB sub-tansaction rollback");
		td_.doRollback = true;
		return;
	}

	cflib::util::Log(lfi_, line_, LogCat::Info | LogCat::Db)("DB tansaction rollback");
	PGresult * res = PQexec(td_.conn, "ROLLBACK");
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		cflib::util::Log(lfi_, line_, LogCat::Warn | LogCat::Db)(
			"DB transaction rollback failed: %1", PQerrorMessage(td_.conn));
	}
	PQclear(res);

	td_.doRollback = false;
	td_.transactionActive = false;
}

bool PSql::exec(const QString & query)
{
	if (td_.doRollback) return false;

	clearResult();

	if (!PQsendQueryParams(td_.conn, query.toUtf8().constData(), 0, NULL, NULL, NULL, NULL, 1)) {
		cflib::util::Log(lfi_, line_, LogCat::Debug | LogCat::Db)("query: %1", query);
		cflib::util::Log(lfi_, line_, LogCat::Warn  | LogCat::Db)("cannot send query: %1", PQerrorMessage(td_.conn));
		return false;
	}

	if (!PQsetSingleRowMode(td_.conn)) {
		cflib::util::Log(lfi_, line_, LogCat::Warn  | LogCat::Db)("cannot set single row mode: %1", PQerrorMessage(td_.conn));
		return false;
	}

	isFirstResult_ = true;
	haveResultInfo_ = false;
	res_ = PQgetResult(td_.conn);
	const ExecStatusType status = PQresultStatus((PGresult *)res_);
	logDebug("res status a: %1", (int)status);

	if (status == PGRES_SINGLE_TUPLE) return true;

	clearResult();

	if (status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK) {
		cflib::util::Log(lfi_, line_, LogCat::Debug | LogCat::Db)("query: %1", query);
		cflib::util::Log(lfi_, line_, LogCat::Warn  | LogCat::Db)("cannot send query: %1", PQerrorMessage(td_.conn));
		return false;
	}
	return true;
}

bool PSql::next()
{
	if (!res_) return false;

	if (!haveResultInfo_) {
		haveResultInfo_ = true;

		resultFieldCount_ = PQnfields((PGresult *)res_);
		if (resultFieldCount_ > MAX_FIELD_COUNT) {
			cflib::util::Log(lfi_, line_, LogCat::Warn | LogCat::Db)(
				"too many columns in result set (got: %1, max: %2)", resultFieldCount_, MAX_FIELD_COUNT);
			clearResult();
			return false;
		}

		for (int i = 0 ; i < resultFieldCount_; ++i) {
			resultFieldTypes_[i] = PQftype((PGresult *)res_, i);
		}
	}

	currentFieldId_ = 0;

	if (isFirstResult_) {
		isFirstResult_ = false;
		return true;
	}

	PQclear((PGresult *)res_);
	res_ = PQgetResult(td_.conn);
	const ExecStatusType status = PQresultStatus((PGresult *)res_);
	logDebug("res status b: %1", (int)status);

	if (status == PGRES_SINGLE_TUPLE) return true;

	clearResult();
	return false;
}

PSql & PSql::operator>>(quint32 val)
{
	QTextStream out(stdout);
	out << "type: " << resultFieldTypes_[currentFieldId_] << endl;
	int len = PQgetlength((PGresult *)res_, 0, currentFieldId_);
	out << "len: " << len << endl;

	if (len == 4) {
		val = qFromBigEndian<quint32>((const uchar *)PQgetvalue((PGresult *)res_, 0, currentFieldId_));
	}

	++currentFieldId_;
	return *this;
}

void PSql::clearResult()
{
	while (res_) {
		PQclear((PGresult *)res_);
		res_ = PQgetResult(td_.conn);
	}
}

}}	// namespace

#include "psql.moc"

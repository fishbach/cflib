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

#include "psql.h"

#include <cflib/util/evtimer.h>
#include <cflib/util/threadverify.h>

#include <libpq-fe.h>

USE_LOG(LogCat::Db)

namespace cflib { namespace db {

namespace {

QAtomicInt connIdCounter(1);

enum PostgresTypes {
	PSql_null = 0,
	PSql_int16,
	PSql_int32,
	PSql_int64,
	PSql_float,
	PSql_double,
	PSql_string,
	PSql_binary,
	PSql_timestampWithTimeZone,
	PSql_lastEntry
};

const char * PostgresTypeNames[] = {
	0,
	"smallint",
	"integer",
	"bigint",
	"real",
	"double precision",
	"text",
	"bytea",
	"timestamp with time zone",
};

Oid typeOids[PSql_lastEntry];
QString connInfo;

const qint64 MsecDelta = -QDateTime::fromMSecsSinceEpoch(0).msecsTo(QDateTime(QDate(2000, 1, 1), QTime(0, 0), Qt::UTC));
const QVector<int> ParamFormats(PSql::MAX_FIELD_COUNT, 1);

union FloatInt {
	float f;
	quint32 i;
};

union DoubleInt {
	double d;
	quint64 i;
};

}

class PSql::ThreadData : public QObject
{
	Q_OBJECT
public:
	const bool isDedicated;
	const int connId;
	PGconn * conn;
	bool transactionActive;
	bool doRollback;
	QList<QByteArray> preparedStatements;
	util::EVTimer * evTimer_;
	uint instanceCount_;

public:
	ThreadData(const QString & connectionParameter = QString(), bool isDedicated = false) :
		isDedicated(isDedicated),
		connId(connIdCounter.fetchAndAddRelaxed(1)),
		conn(0),
		transactionActive(false),
		doRollback(false),
		evTimer_(0),
		instanceCount_(0)
	{
		logDebug("new DB connection: %1", connId);

		if (connectionParameter.isNull() && connInfo.isNull()) {
			logWarn("no connection parameters");
			return;
		}

		conn = PQconnectdb(!connectionParameter.isEmpty() ? connectionParameter.toUtf8().constData() : connInfo.toUtf8().constData());
		if (PQstatus(conn) != CONNECTION_OK) {
			logWarn("cannot connect to database (error: %1)", PQerrorMessage(conn));
			PQfinish(conn);
			conn = 0;
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

QThreadStorage<PSql::ThreadData *> PSql::threadData_;

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

	typeOids[PSql_null] = (Oid)0;
	for (Oid oid = PSql_null + 1 ; oid < PSql_lastEntry ; ++oid) {
		PGresult * res = PQexec(conn, QByteArray("SELECT '") + PostgresTypeNames[oid] + "'::regtype::oid");
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

		typeOids[oid] = (Oid)QByteArray(PQgetvalue(res, 0, 0)).toUInt();

		PQclear(res);
	}

	PQfinish(conn);

	connInfo = connectionParameter;
	return true;
}

void PSql::closeConnection()
{
	threadData_.setLocalData(0);
}

PSql::PSql(const util::LogFileInfo * lfi, int line) :
	PSql(threadData_.hasLocalData() ? *(threadData_.localData()) : (
		threadData_.setLocalData(new ThreadData()), *(threadData_.localData())),
		lfi ? *lfi : ::cflib_util_logFileInfo, line)
{
}

PSql::PSql(const QString & connectionParameter) :
	PSql(*(new ThreadData(connectionParameter, true)), ::cflib_util_logFileInfo, 0)
{
}

PSql::PSql(ThreadData & td, const util::LogFileInfo & lfi, int line) :
	td_(td),
	lfi_(lfi), line_(line),
	instanceName_("i" + QByteArray::number(++td_.instanceCount_)),
	nestedTransaction_(false),
	localTransactionActive_(false),
	isFirstResult_(true),
	res_(0),
	haveResultInfo_(false),
	resultFieldCount_(-1),
	resultFieldTypes_{},
	currentFieldId_(-1),
	lastFieldIsNull_(true),
	prepareUsed_(false),
	isPrepared_(false),
	prepareParamCount_(-1),
	prepareParamTypes_{},
	prepareParamLengths_{},
	prepareParamIsNull_(PSql::MAX_FIELD_COUNT, false)
{
	prepareData_.reserve(1024);
}

PSql::~PSql()
{
	clearResult();
	if (localTransactionActive_) rollback();
	--td_.instanceCount_;
	removePreparedStatement();
	if (td_.isDedicated) delete &td_;
}

void PSql::begin()
{
	if (localTransactionActive_) {
		cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn | LogCat::Db)(
			"begin called with active transaction");
		return;
	}
	localTransactionActive_ = true;

	nestedTransaction_ = td_.transactionActive;
	if (nestedTransaction_) {
		cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Debug | LogCat::Db)("DB sub-transaction start");
		return;
	}

	cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Debug | LogCat::Db)("DB transaction start");
	td_.transactionActive = true;
	watch_.start();

	PGresult * res = PQexec(td_.conn, "BEGIN");
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Critical | LogCat::Db)(
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
		cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn | LogCat::Db)(
			"commit called without active transaction");
		return false;
	}
	localTransactionActive_ = false;

	if (nestedTransaction_) {
		cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Debug | LogCat::Db)("DB sub-transaction commit");
		return true;
	}

	bool ok;
	QElapsedTimer watch;
	watch.start();
	PGresult * res = PQexec(td_.conn, "COMMIT");
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		ok = false;
		cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn | LogCat::Db)(
			"DB transaction commit failed: %1", PQerrorMessage(td_.conn));
	} else {
		ok = true;
		cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Debug | LogCat::Db)(
			"DB transaction commit (%1/%2 msec)", watch.elapsed(), watch_.elapsed());
	}
	PQclear(res);

	// try again to remove prepared statements
	for (const QByteArray & in : td_.preparedStatements) {
		PQclear(PQexec(td_.conn, "DEALLOCATE " + in));
	}
	td_.preparedStatements.clear();

	td_.transactionActive = false;
	return ok;
}

void PSql::rollback()
{
	if (!localTransactionActive_) {
		cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn | LogCat::Db)(
			"rollback called without active transaction");
		return;
	}
	localTransactionActive_ = false;

	if (nestedTransaction_) {
		cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Info | LogCat::Db)("DB sub-tansaction rollback");
		td_.doRollback = true;
		return;
	}

	cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Info | LogCat::Db)("DB tansaction rollback");
	PGresult * res = PQexec(td_.conn, "ROLLBACK");
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn | LogCat::Db)(
			"DB transaction rollback failed: %1", PQerrorMessage(td_.conn));
	}
	PQclear(res);

	// try again to remove prepared statements
	for (const QByteArray & in : td_.preparedStatements) {
		PQclear(PQexec(td_.conn, "DEALLOCATE " + in));
	}
	td_.preparedStatements.clear();

	td_.doRollback = false;
	td_.transactionActive = false;
}

bool PSql::exec(const QString & query)
{
	if (td_.doRollback) return false;

	clearResult();

	lastQuery_ = query.toUtf8();
	if (!PQsendQueryParams(td_.conn, lastQuery_.constData(), 0, NULL, NULL, NULL, NULL, 1)) {
		cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Debug | LogCat::Db)("query: %1", lastQuery_);
		cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn  | LogCat::Db)("cannot send query: %1", PQerrorMessage(td_.conn));
		return false;
	}

	return initResult();
}

bool PSql::execMultiple(const QString & query)
{
	const QByteArray utf8 = query.toUtf8();
	PGresult * res = PQexec(td_.conn, utf8.constData());
	if (PQresultStatus(res) != PGRES_TUPLES_OK && PQresultStatus(res) != PGRES_COMMAND_OK) {
		cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Debug | LogCat::Db)("query: %1", lastQuery_);
		cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn  | LogCat::Db)("cannot send query: %1", PQerrorMessage(td_.conn));
		PQclear(res);
		return false;
	}
	PQclear(res);
	return true;
}

void PSql::prepare(const QByteArray & query)
{
	lastQuery_ = query;
	isPrepared_ = false;
	prepareParamCount_ = 0;
	prepareData_.clear();
}

bool PSql::exec(uint keepFields)
{
	if (lastQuery_.isNull()) {
		cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn | LogCat::Db)(
			"exec called without prepare");
		return false;
	}

	if (td_.doRollback) return false;

	clearResult();

	if (!isPrepared_) {
		isPrepared_ = true;

		removePreparedStatement();
		PGresult * res = PQprepare(td_.conn, instanceName_.constData(),
			lastQuery_.constData(), prepareParamCount_, prepareParamTypes_);
		if (PQresultStatus(res) != PGRES_COMMAND_OK) {
			cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Debug | LogCat::Db)("query: %1", lastQuery_);
			cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn  | LogCat::Db)("cannot prepare query: %1", PQerrorMessage(td_.conn));
			PQclear(res);
			return false;
		}
		PQclear(res);
		prepareUsed_ = true;
	}

	const char * prepareParamValues[MAX_FIELD_COUNT];
	const char * pos = prepareData_.constData();
	for (int i = 0 ; i < prepareParamCount_ ; ++i) {
		prepareParamValues[i] = prepareParamIsNull_[i] ? 0 : pos;
		pos += prepareParamLengths_[i];
	}

	if (!PQsendQueryPrepared(td_.conn, instanceName_.constData(),
		prepareParamCount_, prepareParamValues, prepareParamLengths_, ParamFormats.constData(), 1))
	{
		cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Debug | LogCat::Db)("query: %1", lastQuery_);
		cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn  | LogCat::Db)("cannot send query: %1", PQerrorMessage(td_.conn));
		return false;
	}

	prepareParamCount_ = keepFields;
	int size = 0;
	for (int i = 0 ; i < prepareParamCount_ ; ++i) size += prepareParamLengths_[i];
	prepareData_.resize(size);

	return initResult();
}

bool PSql::next()
{
	if (!res_) return false;

	if (!haveResultInfo_) {
		haveResultInfo_ = true;

		resultFieldCount_ = PQnfields((PGresult *)res_);
		if (resultFieldCount_ > MAX_FIELD_COUNT) {
			cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn | LogCat::Db)(
				"too many fields in result set (got: %1, max: %2)", resultFieldCount_, MAX_FIELD_COUNT);
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

	if (status == PGRES_SINGLE_TUPLE) return true;

	clearResult();
	return false;
}

PSql & PSql::operator<<(float val)
{
	uchar * dest = setParamType(PSql_float, sizeof(float), false);
	if (dest) qToBigEndian<quint32>(FloatInt{val}.i, dest);
	return *this;
}

PSql & PSql::operator<<(double val)
{
	uchar * dest = setParamType(PSql_double, sizeof(double), false);
	if (dest) qToBigEndian<quint64>(DoubleInt{val}.i, dest);
	return *this;
}

PSql & PSql::operator<<(const QDateTime & val)
{
	if (val.isNull()) {
		setParamType(PSql_timestampWithTimeZone, 0, true);
	} else {
		uchar * dest = setParamType(PSql_timestampWithTimeZone, sizeof(qint64), false);
		if (dest) qToBigEndian<qint64>((val.toMSecsSinceEpoch() + MsecDelta) * 1000, dest);
	}
	return *this;
}

PSql & PSql::operator<<(const QByteArray & val)
{
	if (val.isNull()) {
		setParamType(PSql_binary, 0, true);
	} else {
		uchar * dest = setParamType(PSql_binary, val.size(), false);
		if (dest) memcpy(dest, val.constData(), val.size());
	}
	return *this;
}

PSql & PSql::operator<<(const QString & val)
{
	if (val.isNull()) {
		setParamType(PSql_string, 0, true);
	} else {
		const QByteArray utf8 = val.toUtf8();
		uchar * dest = setParamType(PSql_string, utf8.size(), false);
		if (dest) memcpy(dest, utf8.constData(), utf8.size());
	}
	return *this;
}

PSql & PSql::operator<<(const char * val)
{
	if (!val) {
		setParamType(PSql_string, 0, true);
	} else {
		uint len = qstrlen(val);
		uchar * dest = setParamType(PSql_string, len, false);
		if (dest) memcpy(dest, val, len);
	}
	return *this;
}

PSql & PSql::operator<<(Null)
{
	setParamType(PSql_null, 0, true);
	return *this;
}

PSql & PSql::operator>>(float & val)
{
	val = 0.0;
	if (!checkField(PSql_float, sizeof(float))) return *this;
	if (!lastFieldIsNull_) {
		FloatInt fi;
		fi.i = qFromBigEndian<quint32>((const uchar *)PQgetvalue((PGresult *)res_, 0, currentFieldId_));
		val = fi.f;
	}
	++currentFieldId_;
	return *this;
}

PSql & PSql::operator>>(double & val)
{
	val = 0.0;
	if (!checkField(PSql_double, sizeof(double))) return *this;
	if (!lastFieldIsNull_) {
		DoubleInt di;
		di.i = qFromBigEndian<quint64>((const uchar *)PQgetvalue((PGresult *)res_, 0, currentFieldId_));
		val = di.d;
	}
	++currentFieldId_;
	return *this;
}

PSql & PSql::operator>>(QDateTime & val)
{
	val = QDateTime();
	if (!checkField(PSql_timestampWithTimeZone, sizeof(qint64))) return *this;
	if (!lastFieldIsNull_) {
		qint64 rawTime = qFromBigEndian<qint64>((const uchar *)PQgetvalue((PGresult *)res_, 0, currentFieldId_));
		val.setTimeSpec(Qt::UTC);
		val.setMSecsSinceEpoch(rawTime / 1000 - MsecDelta);
	}
	++currentFieldId_;
	return *this;
}

PSql & PSql::operator>>(QByteArray & val)
{
	val = QByteArray();
	if (!checkField(PSql_binary, 0)) return *this;
	if (!lastFieldIsNull_) {
		val = QByteArray(PQgetvalue((PGresult *)res_, 0, currentFieldId_), PQgetlength((PGresult *)res_, 0, currentFieldId_));
	}
	++currentFieldId_;
	return *this;
}

PSql & PSql::operator>>(QString & val)
{
	val = QString();
	if (!checkField(PSql_string, 0)) return *this;
	if (!lastFieldIsNull_) {
		val = QString::fromUtf8(PQgetvalue((PGresult *)res_, 0, currentFieldId_), PQgetlength((PGresult *)res_, 0, currentFieldId_));
	}
	++currentFieldId_;
	return *this;
}

PSql & PSql::operator>>(Null)
{
	if (!checkField(PSql_null, 0)) return *this;
	++currentFieldId_;
	return *this;
}

bool PSql::isNull(uint fieldId)
{
	currentFieldId_ = fieldId;
	return checkField(PSql_null, 0) && lastFieldIsNull_;
}

void PSql::setInt16(qint16 val)
{
	uchar * dest = setParamType(PSql_int16, sizeof(qint16), false);
	if (dest) qToBigEndian<qint16>(val, dest);
}

void PSql::setInt32(qint32 val)
{
	uchar * dest = setParamType(PSql_int32, sizeof(qint32), false);
	if (dest) qToBigEndian<qint32>(val, dest);
}

void PSql::setInt64(qint64 val)
{
	uchar * dest = setParamType(PSql_int64, sizeof(qint64), false);
	if (dest) qToBigEndian<qint64>(val, dest);
}

void PSql::getInt16(qint16 & val)
{
	val = 0;
	if (!checkField(PSql_int16, sizeof(qint16))) return;
	if (!lastFieldIsNull_) {
		val = qFromBigEndian<qint16>((const uchar *)PQgetvalue((PGresult *)res_, 0, currentFieldId_));
	}
	++currentFieldId_;
}

void PSql::getInt32(qint32 & val)
{
	val = 0;
	if (!checkField(PSql_int32, sizeof(qint32))) return;
	if (!lastFieldIsNull_) {
		val = qFromBigEndian<qint32>((const uchar *)PQgetvalue((PGresult *)res_, 0, currentFieldId_));
	}
	++currentFieldId_;
}

void PSql::getInt64(qint64 & val)
{
	val = 0;
	if (!checkField(PSql_int64, sizeof(qint64))) return;
	if (!lastFieldIsNull_) {
		val = qFromBigEndian<qint64>((const uchar *)PQgetvalue((PGresult *)res_, 0, currentFieldId_));
	}
	++currentFieldId_;
}

bool PSql::initResult()
{
	if (!PQsetSingleRowMode(td_.conn)) {
		cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn  | LogCat::Db)("cannot set single row mode: %1", PQerrorMessage(td_.conn));
		return false;
	}

	isFirstResult_ = true;
	haveResultInfo_ = false;
	res_ = PQgetResult(td_.conn);
	const ExecStatusType status = PQresultStatus((PGresult *)res_);

	if (status == PGRES_SINGLE_TUPLE) return true;

	clearResult();

	if (status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK) {
		logDebug("result status: %1", (int)status);
		cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Debug | LogCat::Db)("query: %1", lastQuery_);
		cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn  | LogCat::Db)("cannot get result: %1", PQerrorMessage(td_.conn));
		return false;
	}
	return true;
}

void PSql::clearResult()
{
	while (res_) {
		PQclear((PGresult *)res_);
		res_ = PQgetResult(td_.conn);
	}
}

bool PSql::checkField(int fieldType, int fieldSize)
{
	lastFieldIsNull_ = true;

	if (!res_) {
		cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn | LogCat::Db)(
			"no result available");
		return false;
	}

	if (currentFieldId_ >= resultFieldCount_) {
		cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn | LogCat::Db)(
			"not enough fields in result (got: %1)", resultFieldCount_);
		clearResult();
		return false;
	}

	if (fieldType != PSql_null && resultFieldTypes_[currentFieldId_] != typeOids[fieldType]) {
		cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn | LogCat::Db)(
			"wrong result type (got: %1, want: %2)", resultFieldTypes_[currentFieldId_], typeOids[fieldType]);
		clearResult();
		return false;
	}

	if (PQgetisnull((PGresult *)res_, 0, currentFieldId_) == 1) return true;

	if (fieldSize > 0) {
		const int len = PQgetlength((PGresult *)res_, 0, currentFieldId_);
		if (len != fieldSize) {
			cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn | LogCat::Db)(
				"wrong result size (got: %1, want: %2)", len, fieldSize);
			clearResult();
			return false;
		}
	}

	lastFieldIsNull_ = false;
	return true;
}

uchar * PSql::setParamType(int fieldType, int fieldSize, bool isNull)
{
	if (prepareParamCount_ >= MAX_FIELD_COUNT) {
		cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn | LogCat::Db)(
			"too many fields for prepare statement (max: %1)", MAX_FIELD_COUNT);
		return 0;
	}

	const Oid srcTypeId = typeOids[fieldType];
	Oid & destTypeId = prepareParamTypes_[prepareParamCount_];
	if (!isPrepared_) {
		destTypeId = srcTypeId;
	} else if (destTypeId != srcTypeId) {
		destTypeId = srcTypeId;
		isPrepared_ = false;
	}

	prepareParamLengths_[prepareParamCount_] = fieldSize;

	prepareParamIsNull_[prepareParamCount_] = isNull;

	++prepareParamCount_;

	const int oldSize = prepareData_.size();
	prepareData_.resize(oldSize + fieldSize);
	return (uchar *)prepareData_.constData() + oldSize;
}

void PSql::removePreparedStatement()
{
	if (!prepareUsed_) return;
	prepareUsed_ = false;

	PGresult * res = PQexec(td_.conn, "DEALLOCATE " + instanceName_);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		td_.preparedStatements << instanceName_;
	}
	PQclear(res);
}

}}	// namespace

#include "psql.moc"

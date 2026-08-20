/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "psql.h"

#include <cflib/util/endian.h>
#include <cflib/util/evtimer.h>
#include <cflib/util/threadverify.h>

#include <cstring>
#include <libpq-fe.h>

USE_LOG(LogCat::Db)

namespace cflib::db {

namespace {

AtomicInt connIdCounter(1);

enum PostgresTypes {
    PSql_null = 0,
    PSql_bool,
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
    "boolean",
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
String connInfo;

// PostgreSQL epoch is 2000-01-01 00:00:00 UTC
// Unix epoch is 1970-01-01 00:00:00 UTC
// Difference in milliseconds: 946684800000
const int64 MsecDelta = 946684800000LL;
const int ParamFormats[PSql::MAX_FIELD_COUNT] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

union FloatInt {
    float f;
    uint32 i;
};

union DoubleInt {
    double d;
    uint64 i;
};

}

class PSql::ThreadData
{
    CF_DISABLE_COPY(ThreadData)
public:
    const bool isDedicated;
    PGconn * conn;
    bool transactionActive;
    bool doRollback;
    List<ByteArray> preparedStatements;
    uint instanceCount;

public:
    ThreadData(const String & connectionParameter = String(), bool isDedicated = false) :
        isDedicated(isDedicated),
        conn(nullptr),
        transactionActive(false),
        doRollback(false),
        instanceCount(0),
        connectionParameter_(connectionParameter.isNull() ? connInfo : connectionParameter),
        connId_(connIdCounter.fetchAndAddRelaxed(1)),
        evTimer_(nullptr)
    {
        logDebug("new DB connection: %1", connId_);

        if (connectionParameter_.isNull()) {
            logWarn("no connection parameters");
            return;
        }

        conn = PQconnectdb(connectionParameter_.charPtr());
        if (PQstatus(conn) != CONNECTION_OK) {
            logWarn("cannot connect to database (error: %1)", PQerrorMessage(conn));
            PQfinish(conn);
            conn = 0;
            return;
        }

        if (util::libEVLoop()) {
            evTimer_ = new util::EVTimer(this, &ThreadData::checkConnection);
            evTimer_->start(15);
        }
    }

    ~ThreadData()
    {
        delete evTimer_;
        PQfinish(conn);
        logDebug("DB connection %1 closed", connId_);
    }

    void checkConnection()
    {
        ElapsedTimer watch;
        watch.start();

        PGresult * res = PQexec(conn, "SELECT 1");
        if (PQresultStatus(res) != PGRES_TUPLES_OK) {
            logWarn("DB error on connection %1 check: %2", connId_, PQerrorMessage(conn));
            PQclear(res);
            PQfinish(conn);

            // try reconnect
            conn = PQconnectdb(connectionParameter_.charPtr());
            if (PQstatus(conn) != CONNECTION_OK) {
                logWarn("cannot connect to database (error: %1)", PQerrorMessage(conn));
                PQfinish(conn);
            }
        } else {
            int64 elapsed = watch.elapsed();
            if (elapsed >= 5) logTrace("DB connection %1 responsiveness: %2 msec", connId_, elapsed);
            PQclear(res);
        }
    }

private:
    const String connectionParameter_;
    const int connId_;
    util::EVTimer * evTimer_;
};

const int PSql::MAX_FIELD_COUNT;

bool PSql::setParameter(const String & connectionParameterRef, const String & overrideEnvVar)
{
    if (!connInfo.isNull()) {
        logWarn("Changing the global DB connection parameters does not reconnect existing connections!");
    }
    connInfo = String();

    String connectionParameter = connectionParameterRef;
    if (!overrideEnvVar.isEmpty()) {
        const char * envVal = getenv(overrideEnvVar.charPtr());
        if (envVal) connectionParameter = String(envVal);
    }

    // try connect
    PGconn * conn = PQconnectdb(connectionParameter.toStdString().c_str());
    if (PQstatus(conn) != CONNECTION_OK) {
        logWarn("cannot connect to database (error: %1)", PQerrorMessage(conn));
        PQfinish(conn);
        return false;
    }

    // query oids
    typeOids[PSql_null] = (Oid)0;
    for (Oid oid = PSql_null + 1 ; oid < PSql_lastEntry ; ++oid) {
        ByteArray query = ByteArray("SELECT '") + PostgresTypeNames[oid] + "'::regtype::oid";
        PGresult * res = PQexec(conn, query.charPtr());
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

        typeOids[oid] = (Oid)ByteArray(PQgetvalue(res, 0, 0)).toUInt();

        PQclear(res);
    }

    // log connection info
    PQconninfoOption * conninfo = PQconninfo(conn);
    if (!conninfo) {
        logWarn("cannot get connection info");
    } else {
        Map<ByteArray, ByteArray> vals;
        for (PQconninfoOption * it = conninfo ; it->keyword != NULL ; ++it) {
            if (it->val != NULL) vals[ByteArray(it->keyword)] = ByteArray(it->val);
        }
        PQconninfoFree(conninfo);

        logInfo("connected to psql://%1@%2:%3/%4", vals[ByteArray("user")], vals[ByteArray("host")], vals[ByteArray("port")], vals[ByteArray("dbname")]);
    }

    PQfinish(conn);

    connInfo = connectionParameter;
    return true;
}

String PSql::setDBName(const String & connectionParameter, const String & dbName)
{
    char * errMsg = 0;
    PQconninfoOption * options = PQconninfoParse(connInfo.toStdString().c_str(), &errMsg);
    if (!options) {
        logWarn("cannot parse connection parameter %1 (error: %2)", connectionParameter, errMsg);
        PQfreemem(errMsg);
        return String();
    }

    String newParams = String("dbname=") + dbName;
    for (PQconninfoOption * it = options ; it->keyword != NULL ; ++it) {
        if (it->val != NULL && String(it->keyword) != "dbname") {
            newParams += String(" ") + it->keyword + "=" + it->val;
        }
    }

    PQconninfoFree(options);
    return newParams;
}

void PSql::closeThreadConnection()
{
    delete threadData_;
    threadData_ = nullptr;
}

PSql::PSql(const util::LogFileInfo * lfi, int line) :
    PSql(threadData_ ? *threadData_ : (
        threadData_ = new ThreadData(), *threadData_),
        lfi ? *lfi : ::cflib_util_logFileInfo, line)
{
}

PSql::PSql(const String & connectionParameter) :
    PSql(*(new ThreadData(connectionParameter, true)), ::cflib_util_logFileInfo, 0)
{
}

PSql::PSql(ThreadData & td, const util::LogFileInfo & lfi, int line) :
    td_(td),
    lfi_(lfi), line_(line),
    instanceName_("i" + ByteArray::number((int64)(++td_.instanceCount))),
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
    --td_.instanceCount;
    removePreparedStatement();
    if (td_.isDedicated) delete &td_;
}

void PSql::begin()
{
    if (localTransactionActive_) {
        util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn | LogCat::Db)(
            "begin called with active transaction");
        return;
    }
    localTransactionActive_ = true;

    nestedTransaction_ = td_.transactionActive;
    if (nestedTransaction_) {
        util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Debug | LogCat::Db)("DB sub-transaction start");
        return;
    }

    util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Debug | LogCat::Db)("DB transaction start");
    td_.transactionActive = true;
    watch_.start();

    PGresult * res = PQexec(td_.conn, "BEGIN");
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Critical | LogCat::Db)(
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
        util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn | LogCat::Db)(
            "commit called without active transaction");
        return false;
    }
    localTransactionActive_ = false;

    if (nestedTransaction_) {
        util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Debug | LogCat::Db)("DB sub-transaction commit");
        return true;
    }

    bool ok;
    ElapsedTimer watch;
    watch.start();
    PGresult * res = PQexec(td_.conn, "COMMIT");
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        ok = false;
        util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn | LogCat::Db)(
            "DB transaction commit failed: %1", PQerrorMessage(td_.conn));
    } else {
        ok = true;
        util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Debug | LogCat::Db)(
            "DB transaction commit (%1/%2 msec)", watch.elapsed(), watch_.elapsed());
    }
    PQclear(res);

    // try again to remove prepared statements
    for (const ByteArray & in : td_.preparedStatements) {
        PQclear(PQexec(td_.conn, ("DEALLOCATE " + in).charPtr()));
    }
    td_.preparedStatements.clear();

    td_.transactionActive = false;
    return ok;
}

void PSql::rollback()
{
    if (!localTransactionActive_) {
        util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn | LogCat::Db)(
            "rollback called without active transaction");
        return;
    }
    localTransactionActive_ = false;

    if (nestedTransaction_) {
        util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Info | LogCat::Db)("DB sub-tansaction rollback");
        td_.doRollback = true;
        return;
    }

    util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Info | LogCat::Db)("DB tansaction rollback");
    PGresult * res = PQexec(td_.conn, "ROLLBACK");
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn | LogCat::Db)(
            "DB transaction rollback failed: %1", PQerrorMessage(td_.conn));
    }
    PQclear(res);

    // try again to remove prepared statements
    for (const ByteArray & in : td_.preparedStatements) {
        PQclear(PQexec(td_.conn, ("DEALLOCATE " + in).charPtr()));
    }
    td_.preparedStatements.clear();

    td_.doRollback = false;
    td_.transactionActive = false;
}

bool PSql::exec(const String & query)
{
    if (td_.doRollback) return false;

    if (query.isNull()) {
        util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn | LogCat::Db)("exec called with null query");
        return false;
    }

    clearResult();

    lastQuery_ = query.toUtf8();
    if (!PQsendQueryParams(td_.conn, lastQuery_.charPtr(), 0, NULL, NULL, NULL, NULL, 1)) {
        util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Debug | LogCat::Db)("query: %1", lastQuery_);
        util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn  | LogCat::Db)("cannot send query: %1", PQerrorMessage(td_.conn));
        return false;
    }

    return initResult();
}

bool PSql::execMultiple(const String & query)
{
    if (query.isNull()) {
        util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn | LogCat::Db)("execMultiple called with null query");
        return false;
    }
    const ByteArray utf8 = query.toUtf8();
    PGresult * res = PQexec(td_.conn, utf8.charPtr());
    if (PQresultStatus(res) != PGRES_TUPLES_OK && PQresultStatus(res) != PGRES_COMMAND_OK) {
        util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Debug | LogCat::Db)("query: %1", lastQuery_);
        util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn  | LogCat::Db)("cannot send query: %1", PQerrorMessage(td_.conn));
        PQclear(res);
        return false;
    }
    PQclear(res);
    return true;
}

void PSql::prepare(const ByteArray & query)
{
    lastQuery_ = query;
    isPrepared_ = false;
    prepareParamCount_ = 0;
    prepareData_.clear();
}

bool PSql::exec(uint keepFields)
{
    if (lastQuery_.isNull()) {
        util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn | LogCat::Db)(
            "exec called without prepare");
        return false;
    }

    if (td_.doRollback) return false;

    clearResult();

    if (!isPrepared_) {
        isPrepared_ = true;

        removePreparedStatement();
        PGresult * res = PQprepare(td_.conn, instanceName_.charPtr(),
            lastQuery_.charPtr(), prepareParamCount_, prepareParamTypes_);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Debug | LogCat::Db)("query: %1", lastQuery_);
            util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn  | LogCat::Db)("cannot prepare query: %1", PQerrorMessage(td_.conn));
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

    if (!PQsendQueryPrepared(td_.conn, instanceName_.charPtr(),
        prepareParamCount_, prepareParamValues, prepareParamLengths_, ParamFormats, 1))
    {
        util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Debug | LogCat::Db)("query: %1", lastQuery_);
        util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn  | LogCat::Db)("cannot send query: %1", PQerrorMessage(td_.conn));
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
            util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn | LogCat::Db)(
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
    uint8 * dest = setParamType(PSql_float, sizeof(float), false);
    if (dest) util::writeBE32(dest, FloatInt{val}.i);
    return *this;
}

PSql & PSql::operator<<(double val)
{
    uint8 * dest = setParamType(PSql_double, sizeof(double), false);
    if (dest) util::writeBE64(dest, DoubleInt{val}.i);
    return *this;
}

PSql & PSql::operator<<(const DateTime & val)
{
    if (val.isNull()) {
        setParamType(PSql_timestampWithTimeZone, 0, true);
    } else {
        uint8 * dest = setParamType(PSql_timestampWithTimeZone, sizeof(int64), false);
        if (dest) util::writeBE64(dest, (uint64)((val.toMSecsSinceEpoch() - MsecDelta) * 1000));
    }
    return *this;
}

PSql & PSql::operator<<(const ByteArray & val)
{
    if (val.isNull()) {
        setParamType(PSql_binary, 0, true);
    } else {
        uint8 * dest = setParamType(PSql_binary, val.size(), false);
        if (dest) memcpy(dest, val.constData(), val.size());
    }
    return *this;
}

PSql & PSql::operator<<(const String & val)
{
    if (val.isNull()) {
        setParamType(PSql_string, 0, true);
    } else {
        const ByteArray utf8 = val.toUtf8();
        uint8 * dest = setParamType(PSql_string, utf8.size(), false);
        if (dest) memcpy(dest, utf8.constData(), utf8.size());
    }
    return *this;
}

PSql & PSql::operator<<(const char * val)
{
    if (!val) {
        setParamType(PSql_string, 0, true);
    } else {
        uint len = strlen(val);
        uint8 * dest = setParamType(PSql_string, len, false);
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
        fi.i = util::readBE32((const uint8 *)PQgetvalue((PGresult *)res_, 0, currentFieldId_));
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
        di.i = util::readBE64((const uint8 *)PQgetvalue((PGresult *)res_, 0, currentFieldId_));
        val = di.d;
    }
    ++currentFieldId_;
    return *this;
}

PSql & PSql::operator>>(DateTime & val)
{
    val = DateTime();
    if (!checkField(PSql_timestampWithTimeZone, sizeof(int64))) return *this;
    if (!lastFieldIsNull_) {
        int64 rawTime = (int64)util::readBE64((const uint8 *)PQgetvalue((PGresult *)res_, 0, currentFieldId_));
        val = DateTime::fromMSecsSinceEpoch(rawTime / 1000 + MsecDelta);
    }
    ++currentFieldId_;
    return *this;
}

PSql & PSql::operator>>(ByteArray & val)
{
    val = ByteArray();
    if (!checkField(PSql_binary, 0)) return *this;
    if (!lastFieldIsNull_) {
        val = ByteArray(PQgetvalue((PGresult *)res_, 0, currentFieldId_), PQgetlength((PGresult *)res_, 0, currentFieldId_));
    }
    ++currentFieldId_;
    return *this;
}

PSql & PSql::operator>>(String & val)
{
    val = String();
    if (!checkField(PSql_string, 0)) return *this;
    if (!lastFieldIsNull_) {
        val = String::fromUtf8(PQgetvalue((PGresult *)res_, 0, currentFieldId_), PQgetlength((PGresult *)res_, 0, currentFieldId_));
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

void PSql::setBool(bool val)
{
    uint8 * dest = setParamType(PSql_bool, 1, false);
    if (dest) *dest = val;
}

void PSql::setInt16(int16 val)
{
    uint8 * dest = setParamType(PSql_int16, sizeof(int16), false);
    if (dest) util::writeBE16(dest, (uint16)val);
}

void PSql::setInt32(int32 val)
{
    uint8 * dest = setParamType(PSql_int32, sizeof(int32), false);
    if (dest) util::writeBE32(dest, (uint32)val);
}

void PSql::setInt64(int64 val)
{
    uint8 * dest = setParamType(PSql_int64, sizeof(int64), false);
    if (dest) util::writeBE64(dest, (uint64)val);
}

void PSql::getBool(bool & val)
{
    val = false;
    if (!checkField(PSql_bool, 1)) return;
    if (!lastFieldIsNull_) {
        val = *PQgetvalue((PGresult *)res_, 0, currentFieldId_);
    }
    ++currentFieldId_;
}

void PSql::getInt16(int16 & val)
{
    val = 0;
    if (!checkField(PSql_int16, sizeof(int16))) return;
    if (!lastFieldIsNull_) {
        val = (int16)util::readBE16((const uint8 *)PQgetvalue((PGresult *)res_, 0, currentFieldId_));
    }
    ++currentFieldId_;
}

void PSql::getInt32(int32 & val)
{
    val = 0;
    if (!checkField(PSql_int32, sizeof(int32))) return;
    if (!lastFieldIsNull_) {
        val = (int32)util::readBE32((const uint8 *)PQgetvalue((PGresult *)res_, 0, currentFieldId_));
    }
    ++currentFieldId_;
}

void PSql::getInt64(int64 & val)
{
    val = 0;
    if (!checkField(PSql_int64, sizeof(int64))) return;
    if (!lastFieldIsNull_) {
        val = (int64)util::readBE64((const uint8 *)PQgetvalue((PGresult *)res_, 0, currentFieldId_));
    }
    ++currentFieldId_;
}

bool PSql::initResult()
{
    if (!PQsetSingleRowMode(td_.conn)) {
        util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn  | LogCat::Db)("cannot set single row mode: %1", PQerrorMessage(td_.conn));
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
        util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Debug | LogCat::Db)("query: %1", lastQuery_);
        util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn  | LogCat::Db)("cannot get result: %1", PQerrorMessage(td_.conn));
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
        util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn | LogCat::Db)(
            "no result available");
        return false;
    }

    if (currentFieldId_ >= resultFieldCount_) {
        util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn | LogCat::Db)(
            "not enough fields in result (got: %1)", resultFieldCount_);
        clearResult();
        return false;
    }

    if (fieldType != PSql_null && resultFieldTypes_[currentFieldId_] != typeOids[fieldType]) {
        util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn | LogCat::Db)(
            "wrong result type (got: %1, want: %2)", resultFieldTypes_[currentFieldId_], typeOids[fieldType]);
        clearResult();
        return false;
    }

    if (PQgetisnull((PGresult *)res_, 0, currentFieldId_) == 1) return true;

    if (fieldSize > 0) {
        const int len = PQgetlength((PGresult *)res_, 0, currentFieldId_);
        if (len != fieldSize) {
            util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn | LogCat::Db)(
                "wrong result size (got: %1, want: %2)", len, fieldSize);
            clearResult();
            return false;
        }
    }

    lastFieldIsNull_ = false;
    return true;
}

uint8 * PSql::setParamType(int fieldType, int fieldSize, bool isNull)
{
    if (prepareParamCount_ >= MAX_FIELD_COUNT) {
        util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn | LogCat::Db)(
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
    return (uint8 *)prepareData_.constData() + oldSize;
}

void PSql::removePreparedStatement()
{
    if (!prepareUsed_) return;
    prepareUsed_ = false;

    PGresult * res = PQexec(td_.conn, ("DEALLOCATE " + instanceName_).charPtr());
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        td_.preparedStatements << instanceName_;
    }
    PQclear(res);
}

} // namespace

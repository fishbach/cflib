/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "psql.h"

#include <cflib/base.h>
#include <cflib/util/evtimer.h>
#include <cflib/util/threadverify.h>

#include <libpq-fe.h>

#include <cstring>

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
const cfint64 MsecDelta = 946684800000LL;
const int ParamFormats[PSql::MAX_FIELD_COUNT] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

union FloatInt {
    float f;
    cfuint32 i;
};

union DoubleInt {
    double d;
    cfuint64 i;
};

// Big-endian byte-order helpers
inline void writeBE16(cfuint8 * dest, cfuint16 val)
{
    dest[0] = (cfuint8)(val >> 8);
    dest[1] = (cfuint8)(val);
}

inline void writeBE32(cfuint8 * dest, cfuint32 val)
{
    dest[0] = (cfuint8)(val >> 24);
    dest[1] = (cfuint8)(val >> 16);
    dest[2] = (cfuint8)(val >> 8);
    dest[3] = (cfuint8)(val);
}

inline void writeBE64(cfuint8 * dest, cfuint64 val)
{
    dest[0] = (cfuint8)(val >> 56);
    dest[1] = (cfuint8)(val >> 48);
    dest[2] = (cfuint8)(val >> 40);
    dest[3] = (cfuint8)(val >> 32);
    dest[4] = (cfuint8)(val >> 24);
    dest[5] = (cfuint8)(val >> 16);
    dest[6] = (cfuint8)(val >> 8);
    dest[7] = (cfuint8)(val);
}

inline cfuint16 readBE16(const cfuint8 * src)
{
    return ((cfuint16)src[0] << 8) | (cfuint16)src[1];
}

inline cfuint32 readBE32(const cfuint8 * src)
{
    return ((cfuint32)src[0] << 24) | ((cfuint32)src[1] << 16) |
           ((cfuint32)src[2] << 8)  | (cfuint32)src[3];
}

inline cfuint64 readBE64(const cfuint8 * src)
{
    return ((cfuint64)src[0] << 56) | ((cfuint64)src[1] << 48) |
           ((cfuint64)src[2] << 40) | ((cfuint64)src[3] << 32) |
           ((cfuint64)src[4] << 24) | ((cfuint64)src[5] << 16) |
           ((cfuint64)src[6] << 8)  | (cfuint64)src[7];
}

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
    cfuint instanceCount;

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

        conn = PQconnectdb(connectionParameter_.c_str());
        if (PQstatus(conn) != CONNECTION_OK) {
            logWarn("cannot connect to database (error: %1)", PQerrorMessage(conn));
            PQfinish(conn);
            conn = 0;
            return;
        }

        if (util::libEVLoopOfThread()) {
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
            conn = PQconnectdb(connectionParameter_.c_str());
            if (PQstatus(conn) != CONNECTION_OK) {
                logWarn("cannot connect to database (error: %1)", PQerrorMessage(conn));
                PQfinish(conn);
            }
        } else {
            cfint64 elapsed = watch.elapsed();
            if (elapsed >= 5) logTrace("DB connection %1 responsiveness: %2 msec", connId_, elapsed);
            PQclear(res);
        }
    }

private:
    const String connectionParameter_;
    const int connId_;
    util::EVTimer * evTimer_;
};

thread_local PSql::ThreadData * PSql::threadData_ = nullptr;

const int PSql::MAX_FIELD_COUNT;

bool PSql::setParameter(const String & connectionParameterRef, const String & overrideEnvVar)
{
    if (!connInfo.isNull()) {
        logWarn("Changing the global DB connection parameters does not reconnect existing connections!");
    }
    connInfo = String();

    String connectionParameter = connectionParameterRef;
    if (!overrideEnvVar.isEmpty()) {
        const char * envVal = getenv(overrideEnvVar.c_str());
        if (envVal) connectionParameter = String(envVal);
    }

    // try connect
    PGconn * conn = PQconnectdb(connectionParameter.c_str());
    if (PQstatus(conn) != CONNECTION_OK) {
        logWarn("cannot connect to database (error: %1)", PQerrorMessage(conn));
        PQfinish(conn);
        return false;
    }

    // query oids
    typeOids[PSql_null] = (Oid)0;
    for (Oid oid = PSql_null + 1 ; oid < PSql_lastEntry ; ++oid) {
        ByteArray query = ByteArray("SELECT '") + PostgresTypeNames[oid] + "'::regtype::oid";
        PGresult * res = PQexec(conn, query.constData());
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
    PQconninfoOption * options = PQconninfoParse(connInfo.c_str(), &errMsg);
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
    instanceName_("i" + ByteArray::number((cfint64)(++td_.instanceCount))),
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
    ElapsedTimer watch;
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
    for (const ByteArray & in : td_.preparedStatements) {
        PQclear(PQexec(td_.conn, ("DEALLOCATE " + in).constData()));
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
    for (const ByteArray & in : td_.preparedStatements) {
        PQclear(PQexec(td_.conn, ("DEALLOCATE " + in).constData()));
    }
    td_.preparedStatements.clear();

    td_.doRollback = false;
    td_.transactionActive = false;
}

bool PSql::exec(const String & query)
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

bool PSql::execMultiple(const String & query)
{
    const ByteArray utf8 = query.toUtf8();
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

void PSql::prepare(const ByteArray & query)
{
    lastQuery_ = query;
    isPrepared_ = false;
    prepareParamCount_ = 0;
    prepareData_.clear();
}

bool PSql::exec(cfuint keepFields)
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
        prepareParamCount_, prepareParamValues, prepareParamLengths_, ParamFormats, 1))
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
    cfuint8 * dest = setParamType(PSql_float, sizeof(float), false);
    if (dest) writeBE32(dest, FloatInt{val}.i);
    return *this;
}

PSql & PSql::operator<<(double val)
{
    cfuint8 * dest = setParamType(PSql_double, sizeof(double), false);
    if (dest) writeBE64(dest, DoubleInt{val}.i);
    return *this;
}

PSql & PSql::operator<<(const DateTime & val)
{
    if (val.isNull()) {
        setParamType(PSql_timestampWithTimeZone, 0, true);
    } else {
        cfuint8 * dest = setParamType(PSql_timestampWithTimeZone, sizeof(cfint64), false);
        if (dest) writeBE64(dest, (cfuint64)((val.toMSecsSinceEpoch() - MsecDelta) * 1000));
    }
    return *this;
}

PSql & PSql::operator<<(const ByteArray & val)
{
    if (val.isNull()) {
        setParamType(PSql_binary, 0, true);
    } else {
        cfuint8 * dest = setParamType(PSql_binary, val.size(), false);
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
        cfuint8 * dest = setParamType(PSql_string, utf8.size(), false);
        if (dest) memcpy(dest, utf8.constData(), utf8.size());
    }
    return *this;
}

PSql & PSql::operator<<(const char * val)
{
    if (!val) {
        setParamType(PSql_string, 0, true);
    } else {
        cfuint len = strlen(val);
        cfuint8 * dest = setParamType(PSql_string, len, false);
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
        fi.i = readBE32((const cfuint8 *)PQgetvalue((PGresult *)res_, 0, currentFieldId_));
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
        di.i = readBE64((const cfuint8 *)PQgetvalue((PGresult *)res_, 0, currentFieldId_));
        val = di.d;
    }
    ++currentFieldId_;
    return *this;
}

PSql & PSql::operator>>(DateTime & val)
{
    val = DateTime();
    if (!checkField(PSql_timestampWithTimeZone, sizeof(cfint64))) return *this;
    if (!lastFieldIsNull_) {
        cfint64 rawTime = (cfint64)readBE64((const cfuint8 *)PQgetvalue((PGresult *)res_, 0, currentFieldId_));
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

bool PSql::isNull(cfuint fieldId)
{
    currentFieldId_ = fieldId;
    return checkField(PSql_null, 0) && lastFieldIsNull_;
}

void PSql::setBool(bool val)
{
    cfuint8 * dest = setParamType(PSql_bool, 1, false);
    if (dest) *dest = val;
}

void PSql::setInt16(cfint16 val)
{
    cfuint8 * dest = setParamType(PSql_int16, sizeof(cfint16), false);
    if (dest) writeBE16(dest, (cfuint16)val);
}

void PSql::setInt32(cfint32 val)
{
    cfuint8 * dest = setParamType(PSql_int32, sizeof(cfint32), false);
    if (dest) writeBE32(dest, (cfuint32)val);
}

void PSql::setInt64(cfint64 val)
{
    cfuint8 * dest = setParamType(PSql_int64, sizeof(cfint64), false);
    if (dest) writeBE64(dest, (cfuint64)val);
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

void PSql::getInt16(cfint16 & val)
{
    val = 0;
    if (!checkField(PSql_int16, sizeof(cfint16))) return;
    if (!lastFieldIsNull_) {
        val = (cfint16)readBE16((const cfuint8 *)PQgetvalue((PGresult *)res_, 0, currentFieldId_));
    }
    ++currentFieldId_;
}

void PSql::getInt32(cfint32 & val)
{
    val = 0;
    if (!checkField(PSql_int32, sizeof(cfint32))) return;
    if (!lastFieldIsNull_) {
        val = (cfint32)readBE32((const cfuint8 *)PQgetvalue((PGresult *)res_, 0, currentFieldId_));
    }
    ++currentFieldId_;
}

void PSql::getInt64(cfint64 & val)
{
    val = 0;
    if (!checkField(PSql_int64, sizeof(cfint64))) return;
    if (!lastFieldIsNull_) {
        val = (cfint64)readBE64((const cfuint8 *)PQgetvalue((PGresult *)res_, 0, currentFieldId_));
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

cfuint8 * PSql::setParamType(int fieldType, int fieldSize, bool isNull)
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
    return (cfuint8 *)prepareData_.constData() + oldSize;
}

void PSql::removePreparedStatement()
{
    if (!prepareUsed_) return;
    prepareUsed_ = false;

    PGresult * res = PQexec(td_.conn, ("DEALLOCATE " + instanceName_).constData());
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        td_.preparedStatements << instanceName_;
    }
    PQclear(res);
}

} // namespace

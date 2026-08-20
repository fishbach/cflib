/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "sqlite.h"

#include <cstring>
#include <sqlite3.h>

USE_LOG(LogCat::Db)

namespace cflib::db {

namespace {

// Internal param type tags (not SQLite column type codes)
enum {
    Param_null   = 0,
    Param_int64  = 1,
    Param_double = 2,
    Param_text   = 3,
    Param_blob   = 4,
};

String dbPath;

}

// ---------------------------------------------------------------------------
// ThreadData
// ---------------------------------------------------------------------------

class SQLite::ThreadData
{
    CF_DISABLE_COPY(ThreadData)
public:
    const bool isDedicated;
    sqlite3 *  conn;
    bool       transactionActive;
    bool       doRollback;
    uint       instanceCount;

public:
    ThreadData(const String & connectionParameter = String(), bool isDedicated = false) :
        isDedicated(isDedicated),
        conn(nullptr),
        transactionActive(false),
        doRollback(false),
        instanceCount(0),
        connectionParameter_(connectionParameter.isNull() ? dbPath : connectionParameter)
    {
        if (connectionParameter_.isNull()) {
            logWarn("no connection parameters");
            return;
        }

        int rc = sqlite3_open(connectionParameter_.charPtr(), &conn);
        if (rc != SQLITE_OK) {
            logWarn("cannot open database '%1': %2", connectionParameter_, sqlite3_errmsg(conn));
            sqlite3_close(conn);
            conn = nullptr;
            return;
        }

        // Allow up to 5 seconds of retry on BUSY before giving up
        sqlite3_busy_timeout(conn, 5000);

        logDebug("opened SQLite database: %1", connectionParameter_);
    }

    ~ThreadData()
    {
        sqlite3_close(conn);
        logDebug("closed SQLite database: %1", connectionParameter_);
    }

private:
    const String connectionParameter_;
};

const int SQLite::MAX_FIELD_COUNT;

// ---------------------------------------------------------------------------
// Static methods
// ---------------------------------------------------------------------------

bool SQLite::setParameter(const String & connectionParameterRef, const String & overrideEnvVar)
{
    if (!dbPath.isNull()) {
        logWarn("Changing the global SQLite connection path does not reconnect existing connections!");
    }
    dbPath = String();

    String connectionParameter = connectionParameterRef;
    if (!overrideEnvVar.isEmpty()) {
        const char * envVal = getenv(overrideEnvVar.charPtr());
        if (envVal) connectionParameter = String(envVal);
    }

    // test-open the database
    sqlite3 * testConn = nullptr;
    int rc = sqlite3_open(connectionParameter.charPtr(), &testConn);
    if (rc != SQLITE_OK) {
        logWarn("cannot open database '%1': %2", connectionParameter, sqlite3_errmsg(testConn));
        sqlite3_close(testConn);
        return false;
    }

    logInfo("connected to SQLite: %1 (version %2)", connectionParameter,
            String(sqlite3_libversion()));

    sqlite3_close(testConn);

    dbPath = connectionParameter;
    return true;
}

String SQLite::setDBName(const String & /*connectionParameter*/, const String & dbName)
{
    return dbName;
}

void SQLite::closeThreadConnection()
{
    delete threadData_;
    threadData_ = nullptr;
}

// ---------------------------------------------------------------------------
// Constructors / Destructor
// ---------------------------------------------------------------------------

SQLite::SQLite(const util::LogFileInfo * lfi, int line) :
    SQLite(threadData_ ? *threadData_ : (
        threadData_ = new ThreadData(), *threadData_),
        lfi ? *lfi : ::cflib_util_logFileInfo, line)
{
}

SQLite::SQLite(const String & connectionParameter) :
    SQLite(*(new ThreadData(connectionParameter, true)), ::cflib_util_logFileInfo, 0)
{
}

SQLite::SQLite(ThreadData & td, const util::LogFileInfo & lfi, int line) :
    td_(td),
    lfi_(lfi), line_(line),
    nestedTransaction_(false),
    localTransactionActive_(false),
    stmt_(nullptr),
    preparedStmt_(nullptr),
    stmtIsOwned_(false),
    isFirstResult_(true),
    haveResultInfo_(false),
    resultFieldCount_(-1),
    currentFieldId_(-1),
    lastFieldIsNull_(true),
    isPrepared_(false),
    prepareParamCount_(0),
    prepareParamTypes_{},
    prepareParamLengths_{},
    prepareParamIsNull_(SQLite::MAX_FIELD_COUNT, false)
{
    prepareData_.reserve(1024);
    ++td_.instanceCount;
}

SQLite::~SQLite()
{
    clearResult();
    if (localTransactionActive_) rollback();
    --td_.instanceCount;
    if (preparedStmt_) {
        sqlite3_finalize((sqlite3_stmt *)preparedStmt_);
        preparedStmt_ = nullptr;
    }
    if (td_.isDedicated) delete &td_;
}

// ---------------------------------------------------------------------------
// Transactions
// ---------------------------------------------------------------------------

void SQLite::begin()
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

    char * errMsg = nullptr;
    int rc = sqlite3_exec(td_.conn, "BEGIN", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Critical | LogCat::Db)(
            "starting DB transaction failed: %1", errMsg);
        sqlite3_free(errMsg);
    }
}

bool SQLite::commit()
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

    char * errMsg = nullptr;
    int rc = sqlite3_exec(td_.conn, "COMMIT", nullptr, nullptr, &errMsg);
    bool ok;
    if (rc != SQLITE_OK) {
        ok = false;
        cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn | LogCat::Db)(
            "DB transaction commit failed: %1", errMsg);
        sqlite3_free(errMsg);
    } else {
        ok = true;
        cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Debug | LogCat::Db)("DB transaction commit");
    }

    td_.transactionActive = false;
    return ok;
}

void SQLite::rollback()
{
    if (!localTransactionActive_) {
        cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn | LogCat::Db)(
            "rollback called without active transaction");
        return;
    }
    localTransactionActive_ = false;

    if (nestedTransaction_) {
        cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Info | LogCat::Db)("DB sub-transaction rollback");
        td_.doRollback = true;
        return;
    }

    cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Info | LogCat::Db)("DB transaction rollback");

    char * errMsg = nullptr;
    int rc = sqlite3_exec(td_.conn, "ROLLBACK", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn | LogCat::Db)(
            "DB transaction rollback failed: %1", errMsg);
        sqlite3_free(errMsg);
    }

    td_.doRollback = false;
    td_.transactionActive = false;
}

// ---------------------------------------------------------------------------
// Execution
// ---------------------------------------------------------------------------

bool SQLite::exec(const String & query)
{
    if (td_.doRollback) return false;

    clearResult();

    const ByteArray utf8 = query.toUtf8();
    sqlite3_stmt * newStmt = nullptr;
    int rc = sqlite3_prepare_v2(td_.conn, utf8.constData(), utf8.size(), &newStmt, nullptr);
    if (rc != SQLITE_OK) {
        cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Debug | LogCat::Db)("query: %1", utf8);
        cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn  | LogCat::Db)(
            "cannot prepare query: %1", sqlite3_errmsg(td_.conn));
        return false;
    }

    rc = sqlite3_step(newStmt);
    if (rc == SQLITE_ROW) {
        return initResult(newStmt, true);
    } else if (rc == SQLITE_DONE) {
        sqlite3_finalize(newStmt);
        return true;
    } else {
        cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Debug | LogCat::Db)("query: %1", utf8);
        cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn  | LogCat::Db)(
            "cannot execute query: %1", sqlite3_errmsg(td_.conn));
        sqlite3_finalize(newStmt);
        return false;
    }
}

bool SQLite::execMultiple(const String & query)
{
    if (query.isNull()) {
        cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn  | LogCat::Db)("execMultiple called with null query");
        return false;
    }
    const ByteArray utf8 = query.toUtf8();
    char * errMsg = nullptr;
    int rc = sqlite3_exec(td_.conn, utf8.charPtr(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Debug | LogCat::Db)("query: %1", utf8);
        cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn  | LogCat::Db)(
            "cannot execute multiple: %1", errMsg);
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

void SQLite::prepare(const ByteArray & query)
{
    lastQuery_ = query;
    isPrepared_ = false;
    prepareParamCount_ = 0;
    prepareData_.clear();
}

bool SQLite::exec(uint keepFields)
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

        if (preparedStmt_) {
            sqlite3_finalize((sqlite3_stmt *)preparedStmt_);
            preparedStmt_ = nullptr;
        }

        sqlite3_stmt * newStmt = nullptr;
        int rc = sqlite3_prepare_v2(td_.conn, lastQuery_.constData(), lastQuery_.size(),
                                    &newStmt, nullptr);
        if (rc != SQLITE_OK) {
            cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Debug | LogCat::Db)("query: %1", lastQuery_);
            cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn  | LogCat::Db)(
                "cannot prepare statement: %1", sqlite3_errmsg(td_.conn));
            isPrepared_ = false;
            return false;
        }
        preparedStmt_ = newStmt;
    }

    sqlite3_reset((sqlite3_stmt *)preparedStmt_);

    if (!bindAllParams()) return false;

    int rc = sqlite3_step((sqlite3_stmt *)preparedStmt_);

    // Truncate to kept fields (same logic as PSql: happens after execution)
    prepareParamCount_ = (int)keepFields;
    int keptSize = 0;
    for (int i = 0; i < prepareParamCount_; ++i) keptSize += prepareParamLengths_[i];
    prepareData_.resize(keptSize);

    if (rc == SQLITE_ROW) {
        return initResult(preparedStmt_, false);
    } else if (rc == SQLITE_DONE) {
        return true;
    } else {
        cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Debug | LogCat::Db)("query: %1", lastQuery_);
        cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn  | LogCat::Db)(
            "cannot execute prepared statement: %1", sqlite3_errmsg(td_.conn));
        return false;
    }
}

bool SQLite::next()
{
    if (!stmt_) return false;

    if (!haveResultInfo_) {
        haveResultInfo_ = true;
        resultFieldCount_ = sqlite3_column_count((sqlite3_stmt *)stmt_);
        if (resultFieldCount_ > MAX_FIELD_COUNT) {
            cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn | LogCat::Db)(
                "too many fields in result set (got: %1, max: %2)", resultFieldCount_, MAX_FIELD_COUNT);
            clearResult();
            return false;
        }
    }

    currentFieldId_ = 0;

    if (isFirstResult_) {
        isFirstResult_ = false;
        return true;
    }

    int rc = sqlite3_step((sqlite3_stmt *)stmt_);
    if (rc == SQLITE_ROW) return true;

    clearResult();
    return false;
}

// ---------------------------------------------------------------------------
// Operator<< (bind parameters)
// ---------------------------------------------------------------------------

SQLite & SQLite::operator<<(float val)
{
    setDouble((double)val);
    return *this;
}

SQLite & SQLite::operator<<(double val)
{
    setDouble(val);
    return *this;
}

SQLite & SQLite::operator<<(const DateTime & val)
{
    if (val.isNull()) {
        setParam(Param_null, 0, true);
    } else {
        setInt64(val.toMSecsSinceEpoch());
    }
    return *this;
}

SQLite & SQLite::operator<<(const ByteArray & val)
{
    if (val.isNull()) {
        setParam(Param_null, 0, true);
    } else {
        uint8 * dest = setParam(Param_blob, val.size(), false);
        if (dest) memcpy(dest, val.constData(), val.size());
    }
    return *this;
}

SQLite & SQLite::operator<<(const String & val)
{
    if (val.isNull()) {
        setParam(Param_null, 0, true);
    } else {
        const ByteArray utf8 = val.toUtf8();
        uint8 * dest = setParam(Param_text, utf8.size(), false);
        if (dest) memcpy(dest, utf8.constData(), utf8.size());
    }
    return *this;
}

SQLite & SQLite::operator<<(const char * val)
{
    if (!val) {
        setParam(Param_null, 0, true);
    } else {
        int len = (int)strlen(val);
        uint8 * dest = setParam(Param_text, len, false);
        if (dest) memcpy(dest, val, len);
    }
    return *this;
}

SQLite & SQLite::operator<<(Null)
{
    setParam(Param_null, 0, true);
    return *this;
}

// ---------------------------------------------------------------------------
// Operator>> (read result columns)
// ---------------------------------------------------------------------------

SQLite & SQLite::operator>>(float & val)
{
    val = 0.0f;
    if (!checkField()) return *this;
    if (!lastFieldIsNull_) {
        val = (float)sqlite3_column_double((sqlite3_stmt *)stmt_, currentFieldId_);
    }
    ++currentFieldId_;
    return *this;
}

SQLite & SQLite::operator>>(double & val)
{
    val = 0.0;
    if (!checkField()) return *this;
    if (!lastFieldIsNull_) {
        val = sqlite3_column_double((sqlite3_stmt *)stmt_, currentFieldId_);
    }
    ++currentFieldId_;
    return *this;
}

SQLite & SQLite::operator>>(DateTime & val)
{
    val = DateTime();
    if (!checkField()) return *this;
    if (!lastFieldIsNull_) {
        int64 ms = sqlite3_column_int64((sqlite3_stmt *)stmt_, currentFieldId_);
        val = DateTime::fromMSecsSinceEpoch(ms);
    }
    ++currentFieldId_;
    return *this;
}

SQLite & SQLite::operator>>(ByteArray & val)
{
    val = ByteArray();
    if (!checkField()) return *this;
    if (!lastFieldIsNull_) {
        // For zero-length blobs, sqlite3_column_blob() returns NULL.
        // Use sqlite3_column_bytes() first to distinguish null from empty.
        int bytes = sqlite3_column_bytes((sqlite3_stmt *)stmt_, currentFieldId_);
        if (bytes > 0) {
            const void * data = sqlite3_column_blob((sqlite3_stmt *)stmt_, currentFieldId_);
            val = ByteArray((const char *)data, bytes);
        } else {
            val = ByteArray("");  // empty blob: non-null, zero length
        }
    }
    ++currentFieldId_;
    return *this;
}

SQLite & SQLite::operator>>(String & val)
{
    val = String();
    if (!checkField()) return *this;
    if (!lastFieldIsNull_) {
        const unsigned char * text = sqlite3_column_text((sqlite3_stmt *)stmt_, currentFieldId_);
        int bytes = sqlite3_column_bytes((sqlite3_stmt *)stmt_, currentFieldId_);
        val = String::fromUtf8((const char *)text, bytes);
    }
    ++currentFieldId_;
    return *this;
}

SQLite & SQLite::operator>>(Null)
{
    if (!checkField()) return *this;
    ++currentFieldId_;
    return *this;
}

bool SQLite::isNull(uint fieldId)
{
    currentFieldId_ = (int)fieldId;
    return checkField() && lastFieldIsNull_;
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void SQLite::setInt64(int64 val)
{
    uint8 * dest = setParam(Param_int64, sizeof(int64), false);
    if (dest) memcpy(dest, &val, sizeof(int64));
}

void SQLite::setDouble(double val)
{
    uint8 * dest = setParam(Param_double, sizeof(double), false);
    if (dest) memcpy(dest, &val, sizeof(double));
}

void SQLite::getInt64(int64 & val)
{
    val = 0;
    if (!checkField()) return;
    if (!lastFieldIsNull_) {
        val = sqlite3_column_int64((sqlite3_stmt *)stmt_, currentFieldId_);
    }
    ++currentFieldId_;
}

bool SQLite::initResult(void * newStmt, bool owned)
{
    stmt_          = newStmt;
    stmtIsOwned_   = owned;
    isFirstResult_ = true;
    haveResultInfo_= false;
    resultFieldCount_ = 0;
    currentFieldId_= 0;
    return true;
}

void SQLite::clearResult()
{
    if (!stmt_) return;
    if (stmtIsOwned_) {
        sqlite3_finalize((sqlite3_stmt *)stmt_);
    } else {
        sqlite3_reset((sqlite3_stmt *)stmt_);
    }
    stmt_ = nullptr;
}

bool SQLite::checkField()
{
    lastFieldIsNull_ = true;

    if (!stmt_) {
        cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn | LogCat::Db)(
            "no result available");
        return false;
    }

    if (!haveResultInfo_) {
        haveResultInfo_ = true;
        resultFieldCount_ = sqlite3_column_count((sqlite3_stmt *)stmt_);
    }

    if (currentFieldId_ >= resultFieldCount_) {
        cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn | LogCat::Db)(
            "not enough fields in result (got: %1)", resultFieldCount_);
        clearResult();
        return false;
    }

    if (sqlite3_column_type((sqlite3_stmt *)stmt_, currentFieldId_) == SQLITE_NULL) {
        return true;  // lastFieldIsNull_ stays true
    }

    lastFieldIsNull_ = false;
    return true;
}

bool SQLite::bindAllParams()
{
    sqlite3_stmt * stmt = (sqlite3_stmt *)preparedStmt_;
    sqlite3_clear_bindings(stmt);

    const char * pos = prepareData_.constData();
    for (int i = 0; i < prepareParamCount_; ++i) {
        int idx = i + 1;  // SQLite parameters are 1-indexed
        if (prepareParamIsNull_[i]) {
            sqlite3_bind_null(stmt, idx);
            continue;
        }
        switch (prepareParamTypes_[i]) {
        case Param_int64: {
            int64 v;
            memcpy(&v, pos, sizeof(int64));
            sqlite3_bind_int64(stmt, idx, v);
            break;
        }
        case Param_double: {
            double v;
            memcpy(&v, pos, sizeof(double));
            sqlite3_bind_double(stmt, idx, v);
            break;
        }
        case Param_text:
            sqlite3_bind_text(stmt, idx, pos, prepareParamLengths_[i], SQLITE_TRANSIENT);
            break;
        case Param_blob:
            sqlite3_bind_blob(stmt, idx, pos, prepareParamLengths_[i], SQLITE_TRANSIENT);
            break;
        default:
            sqlite3_bind_null(stmt, idx);
            break;
        }
        pos += prepareParamLengths_[i];
    }
    return true;
}

uint8 * SQLite::setParam(int type, int size, bool isNull)
{
    if (prepareParamCount_ >= MAX_FIELD_COUNT) {
        cflib::util::Log(lfi_, line_ ? line_ : __LINE__, LogCat::Warn | LogCat::Db)(
            "too many fields for prepare statement (max: %1)", MAX_FIELD_COUNT);
        return nullptr;
    }

    prepareParamTypes_[prepareParamCount_]   = type;
    prepareParamLengths_[prepareParamCount_] = size;
    prepareParamIsNull_[prepareParamCount_]  = isNull;
    ++prepareParamCount_;

    const int oldSize = prepareData_.size();
    prepareData_.resize(oldSize + size);
    return (uint8 *)prepareData_.constData() + oldSize;
}

} // namespace

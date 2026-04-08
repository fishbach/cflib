/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/util/log.h>

#include <cflib/base.h>

#define SQLiteConn  cflib::db::SQLite sql (&::cflib_util_logFileInfo, __LINE__)
#define SQLiteConn2 cflib::db::SQLite sql2(&::cflib_util_logFileInfo, __LINE__)
#define SQLiteConn3 cflib::db::SQLite sql3(&::cflib_util_logFileInfo, __LINE__)
#define SQLiteConn4 cflib::db::SQLite sql4(&::cflib_util_logFileInfo, __LINE__)

namespace cflib::db {

/* Attention:
 * SQLite uses one DB connection per thread.
 *
 * SQLite type mapping:
 *   INTEGER  <->  bool, int8, uint8, int16, uint16, int32, uint32, int64, uint64
 *   REAL     <->  float, double
 *   TEXT     <->  String
 *   BLOB     <->  ByteArray
 *   INTEGER  <->  DateTime (milliseconds since Unix epoch)
 *
 * Parameters in SQL strings use ?1, ?2, ... syntax.
 */

class SQLite
{
    CF_DISABLE_COPY(SQLite)
private:
    class ThreadData;

public:
    static const int MAX_FIELD_COUNT = 64;
    struct Null {} null;

    static bool setParameter(const String & connectionParameter, const String & overrideEnvVar = String());
    static String setDBName(const String & connectionParameter, const String & dbName);
    static void closeThreadConnection();

public:
    // This constructor uses the thread specific DB-connection.
    SQLite(const cflib::util::LogFileInfo * lfi, int line);

    // This constructor opens an own DB-connection.
    // If connectionParameter is empty, the default parameters will be used.
    SQLite(const String & connectionParameter = String());

    ~SQLite();

    void begin();
    bool commit();
    void rollback();

    bool exec(const String & query);

    // This can be used to execute many statements in one string.
    bool execMultiple(const String & query);

    void prepare(const ByteArray & query);
    bool exec(uint keepFields = 0);

    bool next();

    inline SQLite & operator<<(bool     val) { setInt64(val ? 1LL : 0LL); return *this; }
    inline SQLite & operator<<(int8   val) { setInt64((int64)val); return *this; }
    inline SQLite & operator<<(uint8  val) { setInt64((int64)val); return *this; }
    inline SQLite & operator<<(int16  val) { setInt64((int64)val); return *this; }
    inline SQLite & operator<<(uint16 val) { setInt64((int64)val); return *this; }
    inline SQLite & operator<<(int32  val) { setInt64((int64)val); return *this; }
    inline SQLite & operator<<(uint32 val) { setInt64((int64)val); return *this; }
    inline SQLite & operator<<(int64  val) { setInt64(val);        return *this; }
    inline SQLite & operator<<(uint64 val) { setInt64((int64)val); return *this; }

    SQLite & operator<<(float  val);
    SQLite & operator<<(double val);
    SQLite & operator<<(const DateTime  & val);
    SQLite & operator<<(const ByteArray & val);
    SQLite & operator<<(const String    & val);
    SQLite & operator<<(const char * val);

    SQLite & operator<<(Null);

    inline SQLite & operator>>(bool     & val) { int64 v = 0; getInt64(v); val = v != 0;   return *this; }
    inline SQLite & operator>>(int8   & val) { int64 v = 0; getInt64(v); val = (int8  )v; return *this; }
    inline SQLite & operator>>(uint8  & val) { int64 v = 0; getInt64(v); val = (uint8 )v; return *this; }
    inline SQLite & operator>>(int16  & val) { int64 v = 0; getInt64(v); val = (int16 )v; return *this; }
    inline SQLite & operator>>(uint16 & val) { int64 v = 0; getInt64(v); val = (uint16)v; return *this; }
    inline SQLite & operator>>(int32  & val) { int64 v = 0; getInt64(v); val = (int32 )v; return *this; }
    inline SQLite & operator>>(uint32 & val) { int64 v = 0; getInt64(v); val = (uint32)v; return *this; }
    inline SQLite & operator>>(int64  & val) { getInt64(val);             return *this; }
    inline SQLite & operator>>(uint64 & val) { int64 v = 0; getInt64(v); val = (uint64)v; return *this; }

    SQLite & operator>>(float      & val);
    SQLite & operator>>(double     & val);
    SQLite & operator>>(DateTime  & val);
    SQLite & operator>>(ByteArray & val);
    SQLite & operator>>(String    & val);

    SQLite & operator>>(Null);

    template<typename T>
    inline T get(uint field) {
        currentFieldId_ = field;
        T val; operator>>(val);
        return val;
    }

    inline bool lastFieldIsNull() const { return lastFieldIsNull_; }
    inline bool isNull() { return isNull(currentFieldId_); }
    bool isNull(uint fieldId);

private:
    SQLite(ThreadData & td, const cflib::util::LogFileInfo & lfi, int line);
    void setInt64 (int64  val);
    void setDouble(double val);
    void getInt64 (int64  & val);
    bool initResult(void * newStmt, bool owned);
    void clearResult();
    bool checkField();
    bool bindAllParams();
    uint8 * setParam(int type, int size, bool isNull);

private:
    static inline thread_local ThreadData * threadData_ = nullptr;
    ThreadData & td_;

    const cflib::util::LogFileInfo & lfi_;
    const int line_;
    bool nestedTransaction_;
    bool localTransactionActive_;

    void * stmt_;
    void * preparedStmt_;
    bool   stmtIsOwned_;

    bool isFirstResult_;
    bool haveResultInfo_;
    int  resultFieldCount_;
    int  currentFieldId_;
    bool lastFieldIsNull_;

    ByteArray lastQuery_;
    bool      isPrepared_;
    int       prepareParamCount_;
    int       prepareParamTypes_[MAX_FIELD_COUNT];
    int       prepareParamLengths_[MAX_FIELD_COUNT];
    List<bool>prepareParamIsNull_;
    ByteArray prepareData_;
};

} // namespace

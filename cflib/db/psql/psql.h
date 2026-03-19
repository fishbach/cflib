/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/util/log.h>

#include <cflib/base.h>

#define PSqlConn  cflib::db::PSql sql (&::cflib_util_logFileInfo, __LINE__)
#define PSqlConn2 cflib::db::PSql sql2(&::cflib_util_logFileInfo, __LINE__)
#define PSqlConn3 cflib::db::PSql sql3(&::cflib_util_logFileInfo, __LINE__)
#define PSqlConn4 cflib::db::PSql sql4(&::cflib_util_logFileInfo, __LINE__)

namespace cflib { namespace db {

/* Attention:
 * PSql uses one DB connection per thread.
 * Therefore it is not possible to have two instances of PSql in one thread having both simultaneous active queries!
 *
 * PostgreSQL type mapping:
 *   16 : boolean                   <->  bool
 *   21 : smallint                  <->  cfint8, cfuint8, cfint16, cfuint16
 *   23 : integer (serial)          <->  cfint32, cfuint32
 *   20 : bigint  (bigserial)       <->  cfint64, cfuint64
 *  700 : real                      <->  float
 *  701 : double precision          <->  double
 *   25 : text                      <->  String
 *   17 : bytea                     <->  CFByteArray
 * 1184 : timestamp with time zone  <->  CFDateTime (UTC)
 *
 * SELECT 21::oid::regtype         -> smallint
 * SELECT 'smallint'::regtype::oid -> 21
 */

class PSql
{
    CF_DISABLE_COPY(PSql)
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
    PSql(const cflib::util::LogFileInfo * lfi, int line);

    // This constructor opens an own DB-connection.
    // If connectionParameter is empty, the default parameters will be used.
    PSql(const String & connectionParameter = String());

    ~PSql();

    void begin();
    bool commit();
    void rollback();

    bool exec(const String & query);

    // This can be used to execute many statements in one string.
    bool execMultiple(const String & query);

    void prepare(const CFByteArray & query);
    bool exec(cfuint keepFields = 0);

    bool next();

    inline PSql & operator<<(bool     val) { setBool (         val); return *this; }
    inline PSql & operator<<(cfint8   val) { setInt16(         val); return *this; }
    inline PSql & operator<<(cfuint8  val) { setInt16((cfint8 )val); return *this; }
    inline PSql & operator<<(cfint16  val) { setInt16(         val); return *this; }
    inline PSql & operator<<(cfuint16 val) { setInt16((cfint16)val); return *this; }
    inline PSql & operator<<(cfint32  val) { setInt32(         val); return *this; }
    inline PSql & operator<<(cfuint32 val) { setInt32((cfint32)val); return *this; }
    inline PSql & operator<<(cfint64  val) { setInt64(         val); return *this; }
    inline PSql & operator<<(cfuint64 val) { setInt64((cfint64)val); return *this; }

    PSql & operator<<(float  val);
    PSql & operator<<(double val);
    PSql & operator<<(const CFDateTime  & val);
    PSql & operator<<(const CFByteArray & val);
    PSql & operator<<(const String    & val);
    PSql & operator<<(const char * val);

    PSql & operator<<(Null);

    inline PSql & operator>>(bool     & val) { getBool (           val); return *this; }
    inline PSql & operator>>(cfint8   & val) { cfint16 val16; getInt16(val16); val = val16; return *this; }
    inline PSql & operator>>(cfuint8  & val) { cfint16 val16; getInt16(val16); val = val16; return *this; }
    inline PSql & operator>>(cfint16  & val) { getInt16(           val); return *this; }
    inline PSql & operator>>(cfuint16 & val) { getInt16((cfint16 &)val); return *this; }
    inline PSql & operator>>(cfint32  & val) { getInt32(           val); return *this; }
    inline PSql & operator>>(cfuint32 & val) { getInt32((cfint32 &)val); return *this; }
    inline PSql & operator>>(cfint64  & val) { getInt64(           val); return *this; }
    inline PSql & operator>>(cfuint64 & val) { getInt64((cfint64 &)val); return *this; }

    PSql & operator>>(float      & val);
    PSql & operator>>(double     & val);
    PSql & operator>>(CFDateTime  & val);
    PSql & operator>>(CFByteArray & val);
    PSql & operator>>(String    & val);

    PSql & operator>>(Null);

    template<typename T>
    inline T get(cfuint field) {
        currentFieldId_ = field;
        T val; operator>>(val);
        return val;
    }

    inline bool lastFieldIsNull() const { return lastFieldIsNull_; }
    inline bool isNull() { return isNull(currentFieldId_); }
    bool isNull(cfuint fieldId);

private:
    PSql(ThreadData & td, const cflib::util::LogFileInfo & lfi, int line);
    void setBool (bool val);
    void setInt16(cfint16 val);
    void setInt32(cfint32 val);
    void setInt64(cfint64 val);
    void getBool (bool & val);
    void getInt16(cfint16 & val);
    void getInt32(cfint32 & val);
    void getInt64(cfint64 & val);
    bool initResult();
    void clearResult();
    bool checkField(int fieldType, int fieldSize);
    cfuint8 * setParamType(int fieldType, int fieldSize, bool isNull);
    void removePreparedStatement();

private:
    static thread_local ThreadData * threadData_;
    ThreadData & td_;

    const cflib::util::LogFileInfo & lfi_;
    const int line_;
    const CFByteArray instanceName_;
    bool nestedTransaction_;
    bool localTransactionActive_;
    CFElapsedTimer watch_;

    bool isFirstResult_;
    void * res_;
    bool haveResultInfo_;
    int resultFieldCount_;
    cfuint resultFieldTypes_[MAX_FIELD_COUNT];
    int currentFieldId_;
    CFByteArray lastQuery_;
    bool lastFieldIsNull_;

    bool prepareUsed_;
    bool isPrepared_;
    int prepareParamCount_;
    cfuint prepareParamTypes_[MAX_FIELD_COUNT];
    int prepareParamLengths_[MAX_FIELD_COUNT];
    CFVector<bool> prepareParamIsNull_;

    CFByteArray prepareData_;
};

}}    // namespace

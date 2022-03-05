/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
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

#pragma once

#include <cflib/util/log.h>

#define PSqlConn  cflib::db::PSql sql (&::cflib_util_logFileInfo, __LINE__)
#define PSqlConn2 cflib::db::PSql sql2(&::cflib_util_logFileInfo, __LINE__)
#define PSqlConn3 cflib::db::PSql sql3(&::cflib_util_logFileInfo, __LINE__)
#define PSqlConn4 cflib::db::PSql sql4(&::cflib_util_logFileInfo, __LINE__)

namespace cflib { namespace db {

/* Attention:
 * PSql uses one DB connection per thread.
 * Therefore it is not possible to have two instances of PSql in one thread having both simultaneous acivate queries!
 *
 * PostgreSQL type mapping:
 * smallint                  <->  bool, qint8, quint8, qint16, quint16
 * integer (serial)          <->  qint32, quint32
 * bigint  (bigserial)       <->  qint64, quint64
 * real                      <->  float
 * double precision          <->  double
 * text                      <->  QString
 * bytea                     <->  QByteArray
 * timestamp with time zone  <->  QDateTime (UTC)
 *
 * SELECT 21::oid::regtype         -> smallint
 * SELECT 'smallint'::regtype::oid -> 21
 */

class PSql
{
	Q_DISABLE_COPY(PSql)
private:
	class ThreadData;

public:
	static const int MAX_FIELD_COUNT = 64;
	struct Null {} null;

	static bool setParameter(const QString & connectionParameter);
	static void closeConnection();

public:
	// This constructor uses the thread specific DB-connection.
	PSql(const cflib::util::LogFileInfo * lfi, int line);

	// This constructor opens an own DB-connection.
	// If connectionParameter is empty, the default parameters will be used.
	PSql(const QString & connectionParameter);

	~PSql();

	void begin();
	bool commit();
	void rollback();

	bool exec(const QString & query);

	// This can be used to execute many statements in one string.
	bool execMultiple(const QString & query);

	void prepare(const QByteArray & query);
	bool exec(uint keepFields = 0);

	bool next();

	inline PSql & operator<<(bool    val) { setInt16(        val); return *this; }
	inline PSql & operator<<(qint8   val) { setInt16(        val); return *this; }
	inline PSql & operator<<(quint8  val) { setInt16((qint8 )val); return *this; }
	inline PSql & operator<<(qint16  val) { setInt16(        val); return *this; }
	inline PSql & operator<<(quint16 val) { setInt16((qint16)val); return *this; }
	inline PSql & operator<<(qint32  val) { setInt32(        val); return *this; }
	inline PSql & operator<<(quint32 val) { setInt32((qint32)val); return *this; }
	inline PSql & operator<<(qint64  val) { setInt64(        val); return *this; }
	inline PSql & operator<<(quint64 val) { setInt64((qint64)val); return *this; }

	PSql & operator<<(float  val);
	PSql & operator<<(double val);
	PSql & operator<<(const QDateTime  & val);
	PSql & operator<<(const QByteArray & val);
	PSql & operator<<(const QString    & val);
	PSql & operator<<(const char * val);

	PSql & operator<<(Null);

	inline PSql & operator>>(bool    & val) { qint16 val16; getInt16(val16); val = val16; return *this; }
	inline PSql & operator>>(qint8   & val) { qint16 val16; getInt16(val16); val = val16; return *this; }
	inline PSql & operator>>(quint8  & val) { qint16 val16; getInt16(val16); val = val16; return *this; }
	inline PSql & operator>>(qint16  & val) { getInt16(          val); return *this; }
	inline PSql & operator>>(quint16 & val) { getInt16((qint16 &)val); return *this; }
	inline PSql & operator>>(qint32  & val) { getInt32(          val); return *this; }
	inline PSql & operator>>(quint32 & val) { getInt32((qint32 &)val); return *this; }
	inline PSql & operator>>(qint64  & val) { getInt64(          val); return *this; }
	inline PSql & operator>>(quint64 & val) { getInt64((qint64 &)val); return *this; }

	PSql & operator>>(float      & val);
	PSql & operator>>(double     & val);
	PSql & operator>>(QDateTime  & val);
	PSql & operator>>(QByteArray & val);
	PSql & operator>>(QString    & val);

	PSql & operator>>(Null);

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
	PSql(ThreadData & td, const cflib::util::LogFileInfo & lfi, int line);
	void setInt16(qint16 val);
	void setInt32(qint32 val);
	void setInt64(qint64 val);
	void getInt16(qint16 & val);
	void getInt32(qint32 & val);
	void getInt64(qint64 & val);
	bool initResult();
	void clearResult();
	bool checkField(int fieldType, int fieldSize);
	uchar * setParamType(int fieldType, int fieldSize, bool isNull);
	void removePreparedStatement();

private:
	static QThreadStorage<ThreadData *> threadData_;
	ThreadData & td_;

	const cflib::util::LogFileInfo & lfi_;
	const int line_;
	const QByteArray instanceName_;
	bool nestedTransaction_;
	bool localTransactionActive_;
	QElapsedTimer watch_;

	bool isFirstResult_;
	void * res_;
	bool haveResultInfo_;
	int resultFieldCount_;
	uint resultFieldTypes_[MAX_FIELD_COUNT];
	int currentFieldId_;
	QByteArray lastQuery_;
	bool lastFieldIsNull_;

	bool prepareUsed_;
	bool isPrepared_;
	int prepareParamCount_;
	uint prepareParamTypes_[MAX_FIELD_COUNT];
	int prepareParamLengths_[MAX_FIELD_COUNT];
	QVector<bool> prepareParamIsNull_;

	QByteArray prepareData_;
};

}}	// namespace

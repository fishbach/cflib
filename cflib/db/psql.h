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

#pragma once

#include <cflib/util/log.h>

#define PSqlConn cflib::db::PSql sql(::cflib_util_logFileInfo, __LINE__)
#define Skip cflib::db::PSql::SkipField()

namespace cflib { namespace db {

namespace { class ThreadData; }

class PSql
{
public:
	static const int MAX_FIELD_COUNT = 64;

	struct SkipField {};

public:
	static bool setParameter(const QString & connectionParameter);
	static void closeConnection();

public:
	PSql(const cflib::util::LogFileInfo & lfi, int line);
	~PSql();

	void begin();
	bool commit();
	void rollback();

	bool exec(const QString & query);

	bool next();

	inline PSql & operator>>(qint8   & val) { qint16  val16; getInt16(          val16); val = val16; return *this; }
	inline PSql & operator>>(quint8  & val) { quint16 val16; getInt16((qint16 &)val16); val = val16; return *this; }
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

	PSql & operator>>(SkipField);

	bool isNull();

private:
	void getInt16(qint16 & val);
	void getInt32(qint32 & val);
	void getInt64(qint64 & val);
	void clearResult();
	bool checkField(int fieldType[], int typeCount, int fieldSize);

private:
	ThreadData & td_;
	const cflib::util::LogFileInfo & lfi_;
	const int line_;
	const bool nestedTransaction_;
	bool localTransactionActive_;
	QElapsedTimer watch_;

	bool isFirstResult_;
	void * res_;
	bool haveResultInfo_;
	int resultFieldCount_;
	uint resultFieldTypes_[MAX_FIELD_COUNT];
	int currentFieldId_;
};

}}	// namespace

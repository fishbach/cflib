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

#include <QtSql>

#define Transaction cflib::db::ScopedTransaction ta(::cflib_util_logFileInfo, __LINE__)
#define execQuery(query) cflib::db::exec((query), ::cflib_util_logFileInfo, __LINE__)
#define execQueryCommit(query) (cflib::db::exec((query), ::cflib_util_logFileInfo, __LINE__) && ta.commit())

namespace cflib { namespace db {

class ScopedTransaction
{
	Q_DISABLE_COPY(ScopedTransaction)
private:
	class ThreadData;
	static QThreadStorage<ThreadData *> threadData_;
	ThreadData & td_;

public:
	ScopedTransaction(const cflib::util::LogFileInfo & lfi, int line);
	~ScopedTransaction();

	QSqlDatabase & db;

	bool commit();

private:
	const cflib::util::LogFileInfo & lfi_;
	const int line_;
	bool nested_;
	bool committed_;
	QElapsedTimer watch_;
	friend void closeDBConnection();
};

void closeDBConnection();

void setParameter(const QString & database, const QString & user, const QString & password);

bool exec(QSqlQuery & query, const cflib::util::LogFileInfo & lfi, int line);

QDateTime getUTC(const QSqlQuery & query, int index);

quint64 lastInsertId();

}}	// namespace

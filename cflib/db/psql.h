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

namespace cflib { namespace db {

namespace { class ThreadData; }

class PSql
{
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

private:
	void clearResult();

private:
	ThreadData & td_;
	const cflib::util::LogFileInfo & lfi_;
	const int line_;
	const bool nestedTransaction_;
	bool localTransactionActive_;
	QElapsedTimer watch_;

	bool isFirstResult_;
	void * res_;
};

}}	// namespace

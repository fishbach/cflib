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

#include <cflib/serialize/serialize.h>

namespace cflib { namespace dao {

class Config
{
	SERIALIZE_CLASS
	SERIALIZE_IS_BASE(Config)
public:
	Config();

	inline void loadFromDB() {
		#ifdef CFLIB_POSTGRESQL
			loadFromDBMySql();
		#else
			loadFromDBPSql();
		#endif
	}
	static const Config & instance() { return *instance_; }

public serialized:
	bool isProduction;
	bool emailsEnabled;
	QString baseURL;

protected:
	virtual void init(const QMap<QString, QString> &) {}

private:
	void loadFromDBMySql();
	void loadFromDBPSql();

private:
	static Config * instance_;
};

}}	// namespace

/* Copyright (C) 2013-2014 Christian Fischbach <cf@cflib.de>
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

#include <cflib/net/rmiservice.h>
#include <dao.h>

class InfoService : public cflib::net::RMIService<QString>
{
	SERIALIZE_CLASS
public:
	InfoService();
	~InfoService();

rmi:
	QString test();
	void async(qint64 i);
	qint64 iTest(qint64 i) { return i; }
	Dao update(const Dao & dao);
	void update(Dao2 & dao);
	void update(Dao3 & dao);
};

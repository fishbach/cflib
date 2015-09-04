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

#include "infoservice.h"

#include <cflib/util/log.h>

USE_LOG(LogCat::Compute)

InfoService::InfoService() :
	RMIService(serializeTypeInfo().typeName)
{
}

InfoService::~InfoService()
{
	stopVerifyThread();
}

QString InfoService::test()
{
	return QString::fromUtf8("hello w\xc3\xb6rld");
}

void InfoService::async(int i)
{
	logInfo("async: %1", i);
}

Dao InfoService::update(const Dao & dao)
{
	Dao rv;
	rv.name = dao.name + "XX";
	rv.number = dao.number + 13;
	return rv;
}

void InfoService::update(Dao2 & dao)
{
	dao.numbers << 3 << 4 << 5;
}

void InfoService::update(Dao3 & dao)
{
	dao.timestamp = QDateTime::currentDateTime();
}

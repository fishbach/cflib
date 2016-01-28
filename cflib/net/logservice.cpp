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

#include "logservice.h"

USE_LOG(LogCat::Http)

namespace cflib { namespace net {

LogService::LogService() :
	RMIServiceBase(serializeTypeInfo().typeName)
{
}

LogService::~LogService()
{
	stopVerifyThread();
}

void LogService::log(const QString & file, int line, cflib::util::LogCategory category, const QString & str)
{
	// remove evil chars
	QByteArray fileBa = file.toUtf8();
	for (int i = 0 ; i < fileBa.length() ; ++i) {
		uchar c = (uchar)fileBa[i];
		if (c < 0x20 || c > 0x7E) fileBa[i] = '_';
	}

	cflib::util::Log(cflib::util::LogFileInfo(fileBa.constData(), LogCat::JS),
		line, category)("%1", str.toUtf8());
}

}}	// namespace

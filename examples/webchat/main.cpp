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

#include <infoservice.h>

#include <cflib/net/apiserver.h>
#include <cflib/net/fileserver.h>
#include <cflib/net/httpserver.h>
#include <cflib/net/logservice.h>
#include <cflib/net/requestlog.h>
#include <cflib/net/wscommmanager.h>
#include <cflib/util/log.h>
#include <cflib/util/unixsignal.h>

#include <QtCore>

using namespace cflib::net;
using namespace cflib::util;

USE_LOG(LogCat::Etc)

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	UnixSignal unixSignal(true);

	Log::start("webchat.log");
	logInfo("webchat started");

	RequestLog requestLog;

	ApiServer  api;
	InfoService infoService; api.registerService(&infoService);
	LogService  logService;  api.registerService(&logService);

	FileServer fs("htdocs");

	WSCommManager<QString> wsCommManager("/ws");

	HttpServer serv;
	serv.registerHandler(requestLog);
	serv.registerHandler(wsCommManager);
	serv.registerHandler(api);
	serv.registerHandler(fs);
	serv.start("127.0.0.1", 8080);

	QTimer::singleShot(5000, qApp, SLOT(quit()));
	int retval = a.exec();
	logInfo("terminating softly with retval: %1", retval);
	return retval;
}

/* Copyright (C) 2013-2017 Christian Fischbach <cf@cflib.de>
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

#include <cflib/net/fileserver.h>
#include <cflib/net/httpserver.h>
#include <cflib/net/logservice.h>
#include <cflib/net/requestlog.h>
#include <cflib/net/rmiserver.h>
#include <cflib/net/wscommmanager.h>
#include <cflib/util/cmdline.h>
#include <cflib/util/log.h>
#include <cflib/util/unixsignal.h>

#include <QtCore>

using namespace cflib::net;
using namespace cflib::util;

USE_LOG(LogCat::Etc)

int showUsage(const QByteArray & executable)
{
	QTextStream(stderr)
		<< ""                                                                              << endl
		<< "Simple Webchat example"                                                        << endl
		<< "------------------------------------"                                          << endl
		<< endl
		<< "To make this example run start the webchat executable and access the url"      << endl
		<< "http://127.0.0.1:8080/ through your browser."                                  << endl
		<< "If you have an out of source build you need to redirect the webchat to"        << endl
		<< "the correct location of the example htdocs directory through:"                 << endl
		<< "./" <<executable << " --htdocs <path to cflib source>/examples/webchat/htdocs" << endl
		<< endl
		<< "Usage: " << executable << " [options]"                                         << endl
		<< "Options:"                                                                      << endl
		<< "  -h, --help     => this help"                                                 << endl
		<< "  -e, --export   => export Classes as Javascrpt classes"                       << endl
		<< "  -f, --htdocs   => set path to htdocs (default htdocs in current directory)"  << endl
		<< endl;
	return 1;
}

int main(int argc, char *argv[])
{
	// parse cmd line
	CmdLine cmdLine(argc, argv);
	Option helpOpt      ('h', "help",  false); cmdLine << helpOpt;
	Option exportOpt    ('e', "export", true); cmdLine << exportOpt;
	Option htdocsPathOpt('f', "htdocs", true); cmdLine << htdocsPathOpt;
	if (!cmdLine.parse() || helpOpt.isSet()) return showUsage(cmdLine.executable());

	QCoreApplication a(argc, argv);
	UnixSignal unixSignal(true);

	Log::start("webchat.log");
	logInfo("webchat started");

	RequestLog requestLog;

	FileServer fs(htdocsPathOpt.value("htdocs"));

	WSCommManager<QString> commMgr("/ws");
	RMIServer<QString> rmiServer(commMgr);
	InfoService infoService; rmiServer.registerService(infoService);
	LogService  logService;  rmiServer.registerService(logService);

	if (exportOpt.isSet()) {
		logInfo("exporting to: %1", exportOpt.value());
		rmiServer.exportTo(exportOpt.value());
		fs.exportTo(exportOpt.value());
		return 0;
	}

	HttpServer serv;
	serv.registerHandler(requestLog);
	serv.registerHandler(commMgr);
	serv.registerHandler(rmiServer);
	serv.registerHandler(fs);
	serv.start("127.0.0.1", 8080);

	int retval = a.exec();
	logInfo("terminating softly with retval: %1", retval);
	return retval;
}

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

#include <cflib/net/httprequest.h>
#include <cflib/net/tcpmanager.h>
#include <cflib/util/cmdline.h>
#include <cflib/util/log.h>
#include <cflib/util/util.h>

using namespace cflib::net;
using namespace cflib::util;

USE_LOG(LogCat::Network)

namespace {

int showUsage(const QByteArray & executable)
{
	QTextStream(stderr)
		<< "Usage: " << executable << " [options] <URL>"             << endl
		<< "Options:"                                                << endl
		<< "  -h, --help => this help"                               << endl
		<< "  -p, --post => do POST request"                         << endl
		<< "  -l, --log <level> => set log level 0 -> all, 7 -> off" << endl;
	return 1;
}

}

int main(int argc, char *argv[])
{
	CmdLine cmdLine(argc, argv);
	Option help   ('h', "help"     ); cmdLine << help;
	Option postOpt('p', "post"     ); cmdLine << postOpt;
	Option logOpt ('l', "log", true); cmdLine << logOpt;
	Arg    url    (false           ); cmdLine << url;
	if (!cmdLine.parse() || help.isSet()) return showUsage(cmdLine.executable());

	QCoreApplication a(argc, argv);

	// start logging
	if (logOpt.isSet()) {
		Log::start("request.log");
		logInfo("request started");
		Log::setLogLevel(logOpt.value().toUInt());
	}

	QByteArray postData;
	if (postOpt.isSet()) {
		QFile in;
		in.open(stdin, QFile::ReadOnly);
		postData = in.readAll();
	}

	TCPManager mgr(1);
	HttpRequest request(mgr);
	request.reply.bind([](int status, const QByteArray & reply) {
		QTextStream(stdout)
			<< "Status: " << status << endl
			<< endl
			<< reply << endl;
		threadSafeExit(status == 200 ? 0 : 1);
	});
	request.start(QString::fromUtf8(url.value()), postData);

	return a.exec();
}
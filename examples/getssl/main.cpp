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

#include <cflib/net/fileserver.h>
#include <cflib/net/httpserver.h>
#include <cflib/net/logservice.h>
#include <cflib/net/requestlog.h>
#include <cflib/net/rmiserver.h>
#include <cflib/net/wscommmanager.h>
#include <cflib/util/cmdline.h>
#include <cflib/util/unixsignal.h>

#include <QtCore>

using namespace cflib::net;
using namespace cflib::util;

USE_LOG(LogCat::Etc)

namespace {

int showUsage(const QByteArray & executable)
{
	QTextStream(stderr)
		<< "Usage: " << executable << " [options]" << endl
		<< "Options:"                              << endl
		<< "  -h, --help => this help"             << endl;
	return 1;
}

}

int main(int argc, char *argv[])
{
	// parse cmd line
	CmdLine cmdLine(argc, argv);
	Option help('h', "help"); cmdLine << help;
	if (!cmdLine.parse() || help.isSet()) return showUsage(cmdLine.executable());

	QCoreApplication a(argc, argv);
	UnixSignal unixSignal(true);


	return a.exec();
}

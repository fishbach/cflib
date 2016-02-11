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

#include <letsencrypt.h>

#include <cflib/crypt/util.h>
#include <cflib/net/httpserver.h>
#include <cflib/net/request.h>
#include <cflib/net/requesthandler.h>
#include <cflib/util/cmdline.h>
#include <cflib/util/unixsignal.h>
#include <cflib/util/util.h>

#include <QtCore>
#include <QtNetwork>

using namespace cflib::crypt;
using namespace cflib::net;
using namespace cflib::util;

namespace {

int showUsage(const QByteArray & executable)
{
	QTextStream(stderr)
		<< "Get certificate from letsencrypt.org"                                  << endl
		<< "Usage: " << executable << " [options] <domains ...>"                   << endl
		<< "Options:"                                                              << endl
		<< "  -h, --help           => this help"                                   << endl
		<< "  -k, --key <key file> => private key file for Let's Encrypt"          << endl
		<< "  -e, --email <email>  => contact email for Let's Encrypt"             << endl
		<< "  -d, --dest <dir>     => certificate destination directory"           << endl
		<< "  -t, --test           => testmode using Let's Encrypt staging server" << endl
		<< "  -f, --force          => forces all registration steps to be done"    << endl;
	return 1;
}

}

int main(int argc, char *argv[])
{
	// parse cmd line
	CmdLine cmdLine(argc, argv);
	Option help    ('h', "help"       ); cmdLine << help;
	Option keyFile ('k', "key",   true); cmdLine << keyFile;
	Option email   ('e', "email", true); cmdLine << email;
	Option dest    ('d', "dest",  true); cmdLine << dest;
	Option test    ('t', "test"       ); cmdLine << test;
	Option force   ('f', "force"      ); cmdLine << force;
	Arg    domains (false, true       ); cmdLine << domains;
	if (!cmdLine.parse() || help.isSet()) return showUsage(cmdLine.executable());

	// create application
	QCoreApplication a(argc, argv);
	UnixSignal unixSignal(true);

	LetsEncrypt letsEncrypt(domains.values(), email.value(),
		keyFile.isSet() ? keyFile.value() : "letsencrypt.key",
		dest.isSet() ? dest.value() + "/" : "",
		test.isSet(), force.isSet());

	letsEncrypt.start();

	return a.exec();
}

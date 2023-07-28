/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
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
		<< ""                                                                        << Qt::endl
		<< "Get certificate from letsencrypt.org"                                    << Qt::endl
		<< "------------------------------------"                                    << Qt::endl
		<< Qt::endl
		<< "For communication with Let's Encrypt a dedicated private key is needed." << Qt::endl
		<< "This key is in RSA PKCS#8 PEM encoded format."                               << Qt::endl
		<< "If you do not have one, a new key will be creatd."                       << Qt::endl
		<< "It's a good idea to backup this key to a secure, protected place."       << Qt::endl
		<< "If a domain validation is necessary, letsencrypt.org will contact"       << Qt::endl
		<< "this client at port 80 (HTTP)." << Qt::endl
		<< Qt::endl
		<< "Usage: " << executable << " [options] <domains ...>"                     << Qt::endl
		<< "Options:"                                                                << Qt::endl
		<< "  -h, --help           => this help"                                     << Qt::endl
		<< "  -k, --key <key file> => private key file for Let's Encrypt"            << Qt::endl
		<< "  -e, --email <email>  => contact email for Let's Encrypt"               << Qt::endl
		<< "  -d, --dest <dir>     => certificate destination directory"             << Qt::endl
		<< "  -t, --test           => testmode using Let's Encrypt staging server"   << Qt::endl
		<< Qt::endl;
	return 1;
}

void qtMessageHandler(QtMsgType type, const QMessageLogContext & context, const QString & msg)
{
	if (type == QtFatalMsg) {
		QTextStream(stderr) << "fatal Qt error at " << context.file << ":" << context.line << " : " << msg << Qt::endl;
		::abort();
	}
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
	Arg    domains (false, true       ); cmdLine << domains;
	if (!cmdLine.parse() || help.isSet()) return showUsage(cmdLine.executable());

	// create application
	QCoreApplication a(argc, argv);
	UnixSignal unixSignal(true);

	qInstallMessageHandler(&qtMessageHandler);

	LetsEncrypt letsEncrypt(domains.values(), email.value(),
		keyFile.isSet() ? keyFile.value() : "letsencrypt.key",
		dest.isSet() ? dest.value() + "/" : "",
		test.isSet());

	letsEncrypt.start();

	return a.exec();
}

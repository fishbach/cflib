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

#include <cflib/crypt/util.h>
#include <cflib/net/httpserver.h>
#include <cflib/net/request.h>
#include <cflib/net/requesthandler.h>
#include <cflib/util/cmdline.h>
#include <cflib/util/unixsignal.h>
#include <cflib/util/util.h>

#include <QtCore>

using namespace cflib::crypt;
using namespace cflib::net;
using namespace cflib::util;

namespace {

int showUsage(const QByteArray & executable)
{
	QTextStream(stderr)
		<< "Usage: " << executable << " [options]"                   << endl
		<< "Get certificate from letsencrypt.org"                    << endl
		<< "Options:"                                                << endl
		<< "  -h, --help           => this help"                     << endl
		<< "  -k, --key <key file> => private key for Let's Encrypt" << endl;
	return 1;
}

class AcmeChallenge : public RequestHandler
{
public:
	AcmeChallenge() {}

protected:
	virtual void handleRequest(const Request & request)
	{
		request.sendText(challenge_);
	}

private:
	const QByteArray challenge_;
};

}

int main(int argc, char *argv[])
{
	// parse cmd line
	CmdLine cmdLine(argc, argv);
	Option help   ('h', "help"     ); cmdLine << help;
	Option keyFile('k', "key", true); cmdLine << keyFile;
	if (!cmdLine.parse() || help.isSet()) return showUsage(cmdLine.executable());

	QCoreApplication a(argc, argv);
	UnixSignal unixSignal(true);

	// start http server on port 80
	AcmeChallenge acmeChallenge;
	HttpServer http(1);
	http.registerHandler(acmeChallenge);
	if (!http.start("0.0.0.0", 8080)) {
		QTextStream(stderr) << "cannot listen on port 80" << endl;
		return 2;
	}

	QString keyFilename = QString::fromUtf8(keyFile.value());
	if (keyFilename.isEmpty()) keyFilename = "letsencrypt.key";

	QByteArray privateKey;
	if (!QFile::exists(keyFilename)) {
		QTextStream(stdout) << "creating private key ...";
		privateKey = createRSAKey(4096);
		if (!writeFile(keyFilename, privateKey)) {
			QTextStream(stderr) << "cannot write private key for Let's Encrypt" << endl;
			return 3;
		}
		QTextStream(stdout) << " done" << endl;
	} else {
		privateKey = readFile(keyFilename);
		if (!checkRSAKey(privateKey)) {
			QTextStream(stderr) << "cannot load private key for Let's Encrypt" << endl;
			return 4;
		}
		QTextStream(stdout) << "private key loaded" << endl;
	}

//	return a.exec();
}

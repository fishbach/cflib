/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include <cflib/crypt/tlscredentials.h>
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
		<< "Usage: " << executable << " [options] <URL>"                                                     << Qt::endl
		<< "Options:"                                                                                        << Qt::endl
		<< "  -h, --help        => this help"                                                                << Qt::endl
		<< "  -p, --post        => do POST request"                                                          << Qt::endl
		<< "  -l, --log <level> => set log level 0 -> all, 7 -> off"                                         << Qt::endl
		<< "  -c, --certs <dir> => loads root certificates from given directory"                             << Qt::endl
		<< Qt::endl
		<< "Get the root ca of a certain domain:"                                                            << Qt::endl
		<< "  openssl s_client -showcerts -servername cflib.de -connect cflib.de:443 < /dev/null"            << Qt::endl
		<< "  paste last certificate to: openssl x509 -noout -text"                                          << Qt::endl
		<< "  curl http://crl.identrust.com/DSTROOTCAX3CRL.crl | openssl crl -inform DER"                    << Qt::endl
		<< "  curl http://apps.identrust.com/roots/dstrootcax3.p7c | openssl pkcs7 -inform DER -print_certs" << Qt::endl;
	return 1;
}

}

int main(int argc, char *argv[])
{
	CmdLine cmdLine(argc, argv);
	Option help     ('h', "help"       ); cmdLine << help;
	Option postOpt  ('p', "post"       ); cmdLine << postOpt;
	Option logOpt   ('l', "log",   true); cmdLine << logOpt;
	Option certsOpt ('c', "certs", true); cmdLine << certsOpt;
	Arg     url     (false             ); cmdLine << url;
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

	if (certsOpt.isSet()) {
		mgr.clientCredentials().loadFromDir(QString::fromUtf8(certsOpt.value()));
		mgr.clientCredentials().activateLoaded(true);
	}

	HttpRequest * request = new HttpRequest(mgr);
	request->reply.bind([](int status, const QByteArray & reply) {
		QTextStream(stdout)
			<< "Status: " << status << Qt::endl
			<< Qt::endl
			<< reply << Qt::endl;
		threadSafeExit(status == 200 ? 0 : 1);
	});
	request->start(QString::fromUtf8(url.value()), postData);

	return a.exec();
}

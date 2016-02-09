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
#include <QtNetwork>

using namespace cflib::crypt;
using namespace cflib::net;
using namespace cflib::util;

namespace {

const QByteArray LetsencryptCA        = "https://devel.hochreute.net";
//const QByteArray LetsencryptCA        = "https://acme-staging.api.letsencrypt.org";
//const QByteArray LetsencryptCA        = "https://acme-v01.api.letsencrypt.org";
const QByteArray LetsencryptAgreement = "https://letsencrypt.org/documents/LE-SA-v1.0.1-July-27-2015.pdf";

int showUsage(const QByteArray & executable)
{
	QTextStream(stderr)
		<< "Get certificate from letsencrypt.org"                         << endl
		<< "Usage: " << executable << " [options] -e <email> <domain>"    << endl
		<< "Options:"                                                     << endl
		<< "  -h, --help           => this help"                          << endl
		<< "  -k, --key <key file> => private key file for Let's Encrypt" << endl
		<< "  -e, --email <email>  => contact email for Let's Encrypt"    << endl;
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

class ReplyHdl : public QObject
{
	Q_OBJECT
public:
	ReplyHdl(QNetworkReply * reply, std::function<void (const QNetworkReply &)> finishCb) :
		reply_(reply), finishCb_(finishCb)
	{
		connect(reply, SIGNAL(finished()), this, SLOT(finished()));
	}

	~ReplyHdl()
	{
		delete reply_;
	}

public slots:
	void finished()
	{
		deleteLater();

		if (reply_->error() != QNetworkReply::NoError) {
			QTextStream(stderr) << "network error: " << reply_->errorString() << endl;
			qApp->exit(5);
			return;
		}

		foreach (const QNetworkReply::RawHeaderPair & p, reply_->rawHeaderPairs()) {
			QTextStream(stdout) << "H: " << p.first << " -> " << p.second << endl;
		}
		QTextStream(stdout) << "B: " << reply_->readAll() << endl;

		finishCb_(*reply_);
	}

private:
	QNetworkReply * reply_;
	std::function<void (const QNetworkReply &)> finishCb_;
};

void acmeRequest(const QByteArray & path, const QByteArray & msg,
	const QByteArray & privateKey, QNetworkAccessManager & netMgr,
	std::function<void (const QNetworkReply &)> finishCb)
{
	QNetworkRequest request;
	request.setUrl(QUrl(LetsencryptCA/* + path*/));
	new ReplyHdl(netMgr.get(request), finishCb);
}

}
#include "main.moc"

int main(int argc, char *argv[])
{
	// parse cmd line
	CmdLine cmdLine(argc, argv);
	Option help   ('h', "help"              ); cmdLine << help;
	Option keyFile('k', "key",   true       ); cmdLine << keyFile;
	Option email  ('e', "email", true, false); cmdLine << email;
	Arg    domain(                     false); cmdLine << domain;
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

	QNetworkAccessManager netMgr;

	acmeRequest("/acme/new-reg",
		"{\"resource\": \"new-reg\", \"contact\": [\"mailto: " + email.value() + "\"], "
		"\"agreement\": \"" + LetsencryptAgreement + "\"}",
		privateKey, netMgr, [&](const QNetworkReply & reply)
	{
	});

	return a.exec();
}

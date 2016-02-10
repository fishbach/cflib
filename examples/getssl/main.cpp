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

const QByteArray LetsencryptAgreement = "https://letsencrypt.org/documents/LE-SA-v1.0.1-July-27-2015.pdf";

bool exitting = false;

int showUsage(const QByteArray & executable)
{
	QTextStream(stderr)
		<< "Get certificate from letsencrypt.org"                                  << endl
		<< "Usage: " << executable << " [options] -e <email> <domain>"             << endl
		<< "Options:"                                                              << endl
		<< "  -h, --help           => this help"                                   << endl
		<< "  -k, --key <key file> => private key file for Let's Encrypt"          << endl
		<< "  -e, --email <email>  => contact email for Let's Encrypt"             << endl
		<< "  -d, --dest <dir>     => certificate destination directory"           << endl
		<< "  -t, --test           => testmode using Let's Encrypt staging server" << endl;
	return 1;
}

class AcmeChallenge : public RequestHandler
{
public:
	AcmeChallenge() {}

	void setFile(const QByteArray & path, const QByteArray & content)
	{
		path_ = path;
		content_ = content;
	}

protected:
	virtual void handleRequest(const Request & request)
	{
		if (request.getUri() == path_) request.sendReply(content_, "text/html", false);
	}

private:
	QByteArray path_;
	QByteArray content_;
};

QByteArray nonce;

class ReplyHdl : public QObject
{
	Q_OBJECT
public:
	ReplyHdl(QNetworkReply * reply, std::function<void (QNetworkReply &)> finishCb) :
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
		if (exitting) return;
		if (reply_->hasRawHeader("Replay-Nonce")) nonce = reply_->rawHeader("Replay-Nonce");
		finishCb_(*reply_);
		deleteLater();
	}

private:
	QNetworkReply * reply_;
	std::function<void (QNetworkReply &)> finishCb_;
};

void acmeRequest(const QByteArray & uri, const QByteArray & msg,
	const QByteArray & privateKey, QNetworkAccessManager * netMgr,
	std::function<void (QNetworkReply &)> finishCb)
{
	QByteArray pubMod;
	QByteArray pubExp;
	rsaPublicModulusExponent(privateKey, pubMod, pubExp);
	pubMod = pubMod.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
	pubExp = pubExp.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);

	QByteArray header =
		"\"alg\": \"RS256\", \"jwk\": {\"e\": \"" + pubExp + "\", "
		"\"kty\": \"RSA\", \"n\": \"" + pubMod + "\"}";
	QByteArray pHeader = "{" + header + ", \"nonce\": \"" + nonce + "\"}";
	pHeader = pHeader.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
	QByteArray msg64 = msg.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
	QByteArray signature = rsaSign(privateKey, pHeader + "." + msg64);
	signature = signature.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);

	QByteArray post =
		"{\"header\": {" + header + "}, \"protected\": \"" + pHeader + "\", "
		"\"payload\": \"" + msg64 + "\", \"signature\": \"" + signature + "\"}";

	QNetworkRequest request;
	request.setUrl(QUrl(uri));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
	new ReplyHdl(netMgr->post(request, post), finishCb);
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
	Option dest   ('d', "dest",  true       ); cmdLine << dest;
	Option test   ('t', "test"              ); cmdLine << test;
	Arg    domain(                     false); cmdLine << domain;
	if (!cmdLine.parse() || help.isSet()) return showUsage(cmdLine.executable());

	// create application
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

	// load / create private key
	QByteArray privateKey;
	{
		QString keyFilename = QString::fromUtf8(keyFile.value());
		if (keyFilename.isEmpty()) keyFilename = "letsencrypt.key";
		if (!QFile::exists(keyFilename)) {
			QTextStream(stdout) << "creating private key for Let's Encrypt ...";
			privateKey = rsaCreateKey(4096);
			if (!writeFile(keyFilename, privateKey)) {
				QTextStream(stderr) << "cannot write private key for Let's Encrypt" << endl;
				return 3;
			}
			QTextStream(stdout) << " done" << endl;
		} else {
			privateKey = readFile(keyFilename);
			if (!rsaCheckKey(privateKey)) {
				QTextStream(stderr) << "cannot load private key for Let's Encrypt" << endl;
				return 4;
			}
			QTextStream(stdout) << "private key loaded" << endl;
		}
	}

	// creating private key for certificate
	const QString destDir = dest.isSet() ? dest.value() + "/" : "";
	QTextStream(stdout) << "creating private key for certificate ...";
	QByteArray certKey = rsaCreateKey(2048);
	QTextStream(stdout) << " done" << endl;

	// starting http client mgr
	QNetworkAccessManager netMgr;
	std::function<void (QNetworkReply &)> finishCb;

	const QByteArray LetsencryptCA = test.isSet() ?
		"https://acme-staging.api.letsencrypt.org" :
		"https://acme-v01.api.letsencrypt.org";

	// get first nonce
	new ReplyHdl(netMgr.head(QNetworkRequest(QUrl(LetsencryptCA + "/directory"))), [&](QNetworkReply &)
	{
		// register key
		acmeRequest(LetsencryptCA + "/acme/new-reg",
			"{\"resource\": \"new-reg\", \"contact\": [\"mailto: " + email.value() + "\"], "
			"\"agreement\": \"" + LetsencryptAgreement + "\"}",
			privateKey, &netMgr, [&](QNetworkReply &)
		{
			QTextStream(stdout) << "public key registered" << endl;

			// request challenge
			acmeRequest(LetsencryptCA + "/acme/new-authz",
				"{\"resource\": \"new-authz\", \"identifier\": {\"type\": \"dns\", \"value\": \"" + domain.value() + "\"}}",
				privateKey, &netMgr, [&](QNetworkReply & reply)
			{
				QRegularExpressionMatch match = QRegularExpression(R"(\{[^{]*"type":"http-01"[^}]*\})")
					.match(QString::fromUtf8(reply.readAll()));
				if (!match.hasMatch()) {
					QTextStream(stderr) << "auth challenge not found" << endl;
					qApp->exit(6);
					return;
				}
				const QString challenge = match.captured();
				match = QRegularExpression(R""("uri":"([^"]+)")"").match(challenge);
				const QByteArray uri = match.captured(1).toUtf8();
				match = QRegularExpression(R""("token":"([^"]+)")"").match(challenge);
				const QByteArray token = match.captured(1).toUtf8();

				QByteArray pubMod;
				QByteArray pubExp;
				rsaPublicModulusExponent(privateKey, pubMod, pubExp);
				pubMod = pubMod.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
				pubExp = pubExp.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
				QByteArray keyHash = sha256("{\"e\":\"" + pubExp + "\",\"kty\":\"RSA\",\"n\":\"" + pubMod + "\"}");
				keyHash = keyHash.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
				QByteArray authData = token + "." + keyHash;

				acmeChallenge.setFile("/.well-known/acme-challenge/" + token, authData);

				QTextStream(stdout) << "got auth token" << endl;

				// activate challenge
				acmeRequest(uri,
					"{\"resource\": \"challenge\", \"keyAuthorization\": \"" + authData + "\"}",
					privateKey, &netMgr, [&, uri](QNetworkReply &)
				{
					finishCb = [&, uri](QNetworkReply & reply) {
						QRegularExpressionMatch match = QRegularExpression(R""("status":"([^"]+)")"")
							.match(QString::fromUtf8(reply.readAll()));
						if (!match.hasMatch()) {
							QTextStream(stderr) << "cannot read status of challenge" << endl;
							qApp->exit(7);
							return;
						}
						const QByteArray status = match.captured(1).toUtf8();

						if (status == "pending") {
							QTextStream(stdout) << "waiting for auth challenge ..." << endl;
							QTimer::singleShot(5000, [&, uri]() {
								new ReplyHdl(netMgr.get(QNetworkRequest(QUrl(uri))), finishCb);
							});
							return;
						}
						if (status != "valid") {
							QTextStream(stderr) << "auth challenge failed" << endl;
							qApp->exit(8);
							return;
						}

						QTextStream(stdout) << "auth challenge succeeded" << endl;

						// create csr
						QByteArray csr = x509CreateCertReq(certKey, QList<QByteArray>() << domain.value());
						csr = csr.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);

						// get certificate
						acmeRequest(LetsencryptCA + "/acme/new-cert",
							"{\"resource\": \"new-cert\", \"csr\": \"" + csr + "\"}",
							privateKey, &netMgr, [&](QNetworkReply & reply)
						{
							if (reply.error() != QNetworkReply::NoError) {
								QTextStream(stderr) << "cannot get certifcate" << endl;
								qApp->exit(9);
								return;
							}
							QByteArray crt = der2pem(reply.readAll(), "CERTIFICATE");

							QTextStream(stdout) << "got certificate" << endl;

							if (!writeFile(destDir + domain.value() + "_key.pem", certKey)) {
								QTextStream(stderr) << "cannot write private key for certificate" << endl;
								qApp->exit(10);
								return;
							}

							if (!writeFile(destDir + domain.value() + "_crt.pem", crt)) {
								QTextStream(stderr) << "cannot write certificate" << endl;
								qApp->exit(11);
								return;
							}

							// get intermediate certificate
							QRegularExpressionMatch match = QRegularExpression(R"(<(http.+)>)")
								.match(QString::fromUtf8(reply.rawHeader("Link")));
							if (!match.hasMatch()) {
								QTextStream(stderr) << "issuer not found" << endl;
								qApp->exit(12);
								return;
							}
							new ReplyHdl(netMgr.get(QNetworkRequest(QUrl(match.captured(1)))), [&](QNetworkReply & reply)
							{
								if (reply.error() != QNetworkReply::NoError) {
									QTextStream(stderr) << "cannot get intermediate certificate" << endl;
									qApp->exit(13);
									return;
								}
								QByteArray crt = der2pem(reply.readAll(), "CERTIFICATE");

								if (!writeFile(destDir + "intermediate_crt.pem", crt)) {
									QTextStream(stderr) << "cannot write intermediate certificate" << endl;
									qApp->exit(11);
									return;
								}

								QTextStream(stdout) << "key and certificates written to " <<
									(destDir.isEmpty() ? "current directory" : destDir) << endl <<
									"done!" << endl;
								qApp->quit();
							});
						});
					};
					new ReplyHdl(netMgr.get(QNetworkRequest(QUrl(uri))), finishCb);
				});
			});
		});
	});

	int rv = a.exec();
	exitting = true;
	return rv;
}

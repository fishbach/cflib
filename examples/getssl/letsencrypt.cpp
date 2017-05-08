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

#include "letsencrypt.h"

#include <cflib/crypt/util.h>
#include <cflib/net/httpserver.h>
#include <cflib/net/request.h>
#include <cflib/net/requesthandler.h>
#include <cflib/util/util.h>

#include <QtNetwork>
#include <functional>

using namespace cflib::crypt;
using namespace cflib::net;
using namespace cflib::util;

namespace {

const QByteArray LetsencryptAgreement = "https://letsencrypt.org/documents/LE-SA-v1.1.1-August-1-2016.pdf";

}

class LetsEncrypt::Impl : public QObject, public RequestHandler
{
	Q_OBJECT
public:
	Impl(const QList<QByteArray> & domains, const QByteArray & email,
		const QByteArray & privateKeyFile, const QByteArray & destDir,
		bool test)
	:
		domains_(domains), domainsToBeAuthenticated_(domains), email_(email),
		privateKeyFile_(privateKeyFile), destDir_(destDir),
		letsencryptCA_(test ?
			"https://acme-staging.api.letsencrypt.org" :
			"https://acme-v01.api.letsencrypt.org"),
		registered_(false),
		authorized_(false),
		needRegister_(false),
		reply_(0),
		http_(1)
	{
		http_.registerHandler(*this);
	}

	~Impl()
	{
		delete reply_;
	}

	void start()
	{
		if (!QFile::exists(privateKeyFile_)) {
			QTextStream(stdout) << "creating private key for Let's Encrypt ...";
			privateKey_ = rsaCreateKey(4096);
			if (!writeFile(privateKeyFile_, privateKey_, QFile::ReadOwner | QFile::WriteOwner)) {
				QTextStream(stderr) << "cannot write private key for Let's Encrypt" << endl;
				qApp->exit(2);
				return;
			}
			QTextStream(stdout) << " done" << endl;
			needRegister_ = true;
		} else {
			privateKey_ = readFile(privateKeyFile_);
			if (!rsaCheckKey(privateKey_)) {
				QTextStream(stderr) << "cannot load private key for Let's Encrypt" << endl;
				qApp->exit(3);
				return;
			}
			QTextStream(stdout) << "private key loaded" << endl;
		}

		QTextStream(stdout) << "creating private key for certificate ...";
		certKey_ = rsaCreateKey(2048);
		QTextStream(stdout) << " done" << endl;

		getNonce();
	}

	virtual bool event(QEvent * e)
	{
		if (e->type() == QEvent::User) QTimer::singleShot(1000, this, &Impl::checkChallenge);
		return QObject::event(e);
	}

protected:
	virtual void handleRequest(const Request & request)
	{
		if (request.getUri() == httpPath_) {
			request.sendReply(httpContent_, "text/html", false);

			QCoreApplication::postEvent(this, new QEvent(QEvent::User));
		}
	}

private slots:
	void finished()
	{
		if (reply_->hasRawHeader("Replay-Nonce")) nonce_ = reply_->rawHeader("Replay-Nonce");
		finishCb_(*reply_);
	}

private:
	void getNonce()
	{
		hdl(netMgr_.head(QNetworkRequest(QUrl(letsencryptCA_ + "/directory"))), [&](QNetworkReply &)
		{
			if (nonce_.isEmpty()) {
				QTextStream(stderr) << "cannot get nonce" << endl;
				qApp->exit(4);
				return;
			}
			QTextStream(stdout) << "communication started" << endl;

			if (needRegister_) registerKey();
			else               getCertificate();
		});
	}

	void registerKey()
	{
		if (email_.isEmpty()) {
			QTextStream(stderr) << "need email parameter to register key" << endl;
			qApp->exit(15);
			return;
		}

		registered_ = true;
		acmeRequest(letsencryptCA_ + "/acme/new-reg",
			"{\"resource\": \"new-reg\", \"contact\": [\"mailto: " + email_ + "\"], "
			"\"agreement\": \"" + LetsencryptAgreement + "\"}",
			[&](QNetworkReply &)
		{
			QTextStream(stdout) << "public key registered" << endl;
			startAuth();
		});
	}

	void startAuth()
	{
		authorized_ = true;
		if (!http_.isRunning() && !http_.start("0.0.0.0", 80)) {
			QTextStream(stderr) << "cannot listen on port 80" << endl;
			qApp->exit(5);
			return;
		}
		requestChallenge();
	}

	void requestChallenge()
	{
		acmeRequest(letsencryptCA_ + "/acme/new-authz",
			"{\"resource\": \"new-authz\", \"identifier\": {\"type\": \"dns\", "
			"\"value\": \"" + domainsToBeAuthenticated_.first() + "\"}}",
			[&](QNetworkReply & reply)
		{
			const QByteArray content = reply.readAll();
			if (content.contains("urn:acme:error:unauthorized") && !registered_) {
				registerKey();
				return;
			}

			QRegularExpressionMatch match = QRegularExpression(R"(\{[^{]*"type"\s*:\s*"http-01"[^}]*\})")
				.match(QString::fromUtf8(content));
			if (!match.hasMatch()) {
				QTextStream(stderr) << "auth challenge not found for domain: " << domainsToBeAuthenticated_.first() << endl;
				qApp->exit(6);
				return;
			}
			const QString challenge = match.captured();
			match = QRegularExpression(R"R("uri"\s*:\s*"([^"]+)")R").match(challenge);
			checkUri_ = match.captured(1).toUtf8();
			match = QRegularExpression(R"R("token"\s*:\s*"([^"]+)")R").match(challenge);
			const QByteArray token = match.captured(1).toUtf8();

			QByteArray pubMod;
			QByteArray pubExp;
			rsaPublicModulusExponent(privateKey_, pubMod, pubExp);
			pubMod = pubMod.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
			pubExp = pubExp.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
			QByteArray keyHash = sha256("{\"e\":\"" + pubExp + "\",\"kty\":\"RSA\",\"n\":\"" + pubMod + "\"}");
			keyHash = keyHash.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
			QByteArray authData = token + "." + keyHash;

			httpPath_    = "/.well-known/acme-challenge/" + token;
			httpContent_ = authData;

			QTextStream(stdout) << "got auth token" << endl;

			activateChallenge(authData);
		});
	}

	void activateChallenge(const QByteArray & authData)
	{
		acmeRequest(checkUri_,
			"{\"resource\": \"challenge\", \"keyAuthorization\": \"" + authData + "\"}",
			[&](QNetworkReply &)
		{
			// the incoming HTTP request will trigger checkChallenge()
		});
	}

	void checkChallenge()
	{
		hdl(netMgr_.get(QNetworkRequest(QUrl(checkUri_))), [&](QNetworkReply & reply)
		{
				QRegularExpressionMatch match = QRegularExpression("\"status\"\\s*:\\s*\"([^\"]+)\"")
					.match(QString::fromUtf8(reply.readAll()));
				if (!match.hasMatch()) {
					QTextStream(stderr) << "cannot read status of challenge" << endl;
					qApp->exit(7);
					return;
				}
				const QByteArray status = match.captured(1).toUtf8();

				if (status == "pending") {
					QTextStream(stdout) << "waiting for auth challenge ..." << endl;
					QTimer::singleShot(5000, [this]() { checkChallenge(); });
					return;
				}
				if (status != "valid") {
					QTextStream(stderr) << "auth challenge failed" << endl;
					qApp->exit(8);
					return;
				}

				QTextStream(stdout) << "auth challenge succeeded for: " << domainsToBeAuthenticated_.first() << endl;

				domainsToBeAuthenticated_.takeFirst();
				if (!domainsToBeAuthenticated_.isEmpty()) {
					requestChallenge();
				} else {
					getCertificate();
				}
		});
	}

	void getCertificate()
	{
		// create csr
		QByteArray csr = x509CreateCertReq(certKey_, domains_);
		csr = csr.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);

		// get certificate
		acmeRequest(letsencryptCA_ + "/acme/new-cert",
			"{\"resource\": \"new-cert\", \"csr\": \"" + csr + "\"}",
			[&](QNetworkReply & reply)
		{
			const QByteArray content = reply.readAll();

			if (reply.error() != QNetworkReply::NoError) {
				if (content.contains("urn:acme:error:unauthorized") && !authorized_) {
					startAuth();
					return;
				}
				QTextStream(stderr) << "cannot get certifcate" << endl;
				qApp->exit(9);
				return;
			}
			QByteArray crt = der2pem(content, "CERTIFICATE");

			QTextStream(stdout) << "got certificate" << endl;

			if (!writeFile(destDir_ + domains_.first() + "_key.pem", certKey_, QFile::ReadOwner | QFile::WriteOwner)) {
				QTextStream(stderr) << "cannot write private key for certificate" << endl;
				qApp->exit(10);
				return;
			}

			if (!writeFile(destDir_ + domains_.first() + "_crt.pem", crt)) {
				QTextStream(stderr) << "cannot write certificate" << endl;
				qApp->exit(11);
				return;
			}

			QRegularExpressionMatch match = QRegularExpression(R"(<(http.+)>)")
				.match(QString::fromUtf8(reply.rawHeader("Link")));
			if (!match.hasMatch()) {
				QTextStream(stderr) << "issuer not found" << endl;
				qApp->exit(12);
				return;
			}

			getIntermediateCert(match.captured(1));
		});
	}

	void getIntermediateCert(const QString & url)
	{
		hdl(netMgr_.get(QNetworkRequest(QUrl(url))), [&](QNetworkReply & reply)
		{
			if (reply.error() != QNetworkReply::NoError) {
				QTextStream(stderr) << "cannot get intermediate certificate" << endl;
				qApp->exit(13);
				return;
			}
			QByteArray crt = der2pem(reply.readAll(), "CERTIFICATE");

			if (!writeFile(destDir_ + "intermediate_crt.pem", crt)) {
				QTextStream(stderr) << "cannot write intermediate certificate" << endl;
				qApp->exit(14);
				return;
			}

			QTextStream(stdout) << "key and certificates written to " <<
				(destDir_.isEmpty() ? "current directory" : destDir_) << endl <<
				"done!" << endl;
			qApp->quit();
		});
	}

	inline void hdl(QNetworkReply * reply, std::function<void (QNetworkReply &)> cb)
	{
		if (reply_) reply_->deleteLater();
		reply_ = reply;
		finishCb_ = cb;
		connect(reply_, &QNetworkReply::finished, this, &Impl::finished);
	}

	void acmeRequest(const QByteArray & uri, const QByteArray & msg,
		std::function<void (QNetworkReply &)> finishCb)
	{
		QByteArray pubMod;
		QByteArray pubExp;
		rsaPublicModulusExponent(privateKey_, pubMod, pubExp);
		pubMod = pubMod.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
		pubExp = pubExp.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);

		QByteArray header =
			"\"alg\": \"RS256\", \"jwk\": {\"e\": \"" + pubExp + "\", "
			"\"kty\": \"RSA\", \"n\": \"" + pubMod + "\"}";
		QByteArray pHeader = "{" + header + ", \"nonce\": \"" + nonce_ + "\"}";
		pHeader = pHeader.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
		QByteArray msg64 = msg.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
		QByteArray signature = rsaSign(privateKey_, pHeader + "." + msg64);
		signature = signature.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);

		QByteArray post =
			"{\"header\": {" + header + "}, \"protected\": \"" + pHeader + "\", "
			"\"payload\": \"" + msg64 + "\", \"signature\": \"" + signature + "\"}";

		QNetworkRequest request;
		request.setUrl(QUrl(uri));
		request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
		hdl(netMgr_.post(request, post), finishCb);
	}

private:
	const QList<QByteArray> domains_;
	QList<QByteArray> domainsToBeAuthenticated_;
	const QByteArray email_;
	const QByteArray privateKeyFile_;
	const QByteArray destDir_;
	const QByteArray letsencryptCA_;

	bool registered_;
	bool authorized_;

	QByteArray privateKey_;
	bool needRegister_;
	QByteArray certKey_;
	QByteArray nonce_;
	QByteArray checkUri_;

	QNetworkAccessManager netMgr_;
	QNetworkReply * reply_;
	std::function<void (QNetworkReply &)> finishCb_;

	HttpServer http_;
	QByteArray httpPath_;
	QByteArray httpContent_;
};
#include "letsencrypt.moc"

LetsEncrypt::LetsEncrypt(const QList<QByteArray> & domains, const QByteArray & email,
	const QByteArray & privateKeyFile, const QByteArray & destDir,
	bool test)
:
	impl_(new Impl(domains, email, privateKeyFile, destDir, test))
{
}

LetsEncrypt::~LetsEncrypt()
{
	delete impl_;
}

void LetsEncrypt::start()
{
	impl_->start();
}

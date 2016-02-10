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

#include "letsencrypt.h"

#include <cflib/crypt/util.h>
#include <cflib/net/httpserver.h>
#include <cflib/net/request.h>
#include <cflib/net/requesthandler.h>
#include <cflib/util/util.h>

#include <QtNetwork>

using namespace cflib::crypt;
using namespace cflib::net;
using namespace cflib::util;

namespace {

const QByteArray LetsencryptAgreement = "https://letsencrypt.org/documents/LE-SA-v1.0.1-July-27-2015.pdf";

}

class LetsEncrypt::Impl : public QObject, public RequestHandler
{
	Q_OBJECT
public:
	Impl(const QList<QByteArray> & domains, const QByteArray & email,
		const QByteArray & privateKeyFile, const QByteArray & destDir,
		bool test, bool force)
	:
		domains_(domains), email_(email),
		privateKeyFile_(privateKeyFile), destDir_(destDir),
		letsencryptCA_(test ?
			"https://acme-staging.api.letsencrypt.org" :
			"https://acme-v01.api.letsencrypt.org"),
		needRegister_(force),
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
			if (!writeFile(privateKeyFile_, privateKey_)) {
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

protected:
	virtual void handleRequest(const Request & request)
	{
		if (request.getUri() == httpPath_) {
			request.sendReply(httpContent_, "text/html", false);
			httpAction_();
		}
	}

private slots:
	void finished()
	{
		if (reply_->hasRawHeader("Replay-Nonce")) nonce_ = reply_->rawHeader("Replay-Nonce");
		finishCb_(*reply_);
		reply_->deleteLater();
		reply_ = 0;
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
		acmeRequest(letsencryptCA_ + "/acme/new-reg",
			"{\"resource\": \"new-reg\", \"contact\": [\"mailto: " + email_ + "\"], "
			"\"agreement\": \"" + LetsencryptAgreement + "\"}",
			[&](QNetworkReply &)
		{
			QTextStream(stdout) << "public key registered" << endl;
			requestChallenge();
		});
	}

	void requestChallenge()
	{
		if (!http_.start("0.0.0.0", 8080)) {
			QTextStream(stderr) << "cannot listen on port 80" << endl;
			qApp->exit(5);
			return;
		}

	}

	void getCertificate()
	{
	}

	inline void hdl(QNetworkReply * reply, std::function<void (QNetworkReply &)> cb)
	{
		finishCb_ = cb;
		reply_ = reply;
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
	const QByteArray email_;
	const QByteArray privateKeyFile_;
	const QByteArray destDir_;
	const QByteArray letsencryptCA_;

	QByteArray privateKey_;
	bool needRegister_;
	QByteArray certKey_;
	QByteArray nonce_;

	QNetworkAccessManager netMgr_;
	QNetworkReply * reply_;
	std::function<void (QNetworkReply &)> finishCb_;

	HttpServer http_;
	QByteArray httpPath_;
	QByteArray httpContent_;
	std::function<void ()> httpAction_;
};
#include "letsencrypt.moc"

LetsEncrypt::LetsEncrypt(const QList<QByteArray> & domains, const QByteArray & email,
	const QByteArray & privateKeyFile, const QByteArray & destDir,
	bool test, bool force)
:
	impl_(new Impl(domains, email, privateKeyFile, destDir, test, force))
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

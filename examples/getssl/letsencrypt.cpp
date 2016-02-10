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

#include <QtNetwork>

namespace {

const QByteArray LetsencryptAgreement = "https://letsencrypt.org/documents/LE-SA-v1.0.1-July-27-2015.pdf";

}

class LetsEncrypt::Impl : public QObject
{
	Q_OBJECT
public:
	Impl(const QList<QByteArray> & domains, const QByteArray & email,
		const QByteArray & privateKeyFile, const QByteArray & destDir,
		bool test, bool force)
	:
		reply_(0)
	{
	}

	~Impl()
	{
		delete reply_;
	}

	void start()
	{
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
	QByteArray nonce_;
	QNetworkReply * reply_;
	std::function<void (QNetworkReply &)> finishCb_;
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

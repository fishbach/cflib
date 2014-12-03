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

#include "mailer.h"

#include <cflib/util/log.h>
#include <cflib/util/util.h>

USE_LOG(LogCat::Etc)

namespace cflib { namespace util {

Mailer * Mailer::instance_ = 0;

Mailer::Mailer() :
	ThreadVerify("Mailer", ThreadVerify::Qt),
	process_(0)
{
	if (!instance_) {
		instance_ = this;
		isFirstInstance_ = true;
	} else isFirstInstance_ = false;

	QStringList paths = QProcessEnvironment::systemEnvironment().value("PATH").split(':');
	paths << "/usr/lib/" << "/usr/sbin/";
	foreach (QString path, paths) {
		if (!path.endsWith('/')) path += '/';
		path += "sendmail";
		QFileInfo info(path);
		if (info.isExecutable()) {
			sendmailPath_ = info.canonicalFilePath();
			break;
		}
	}
	if (sendmailPath_.isNull()) {
		logWarn("cannot find sendmail executable (searched: %1)", paths.join(", "));
	} else {
		logDebug("found sendmail at: %1", sendmailPath_);
	}

	initThreadData();
}

Mailer::~Mailer()
{
	stopVerifyThread();
	if (isFirstInstance_) instance_ = 0;
}

void Mailer::initThreadData()
{
	if (!verifyThreadCall(&Mailer::initThreadData)) return;
	logFunctionTrace

	process_ = new QProcess;
	connect(
		process_, SIGNAL(finished(int, QProcess::ExitStatus)),
		this,     SLOT  (finished(int, QProcess::ExitStatus)),
	Qt::DirectConnection);
	connect(
		process_, SIGNAL(error(QProcess::ProcessError)),
		this,     SLOT  (error(QProcess::ProcessError)),
	Qt::DirectConnection);
}

void Mailer::deleteThreadData()
{
	if (process_->state() != QProcess::NotRunning) {
		logWarn("mail process still running");
		process_->kill();
	}
	delete process_;
	process_ = 0;
}

void Mailer::finished(int exitCode, QProcess::ExitStatus exitStatus)
{
	logFunctionTrace

	Mail mail = queue_.takeFirst();

	// check status
	if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
		logInfo("email to %1 sent", mail.to);
	} else {
		logWarn("could not send email to %1", mail.to);
	}

	// check output
	QByteArray output;
	output += process_->readAllStandardOutput();
	output += process_->readAllStandardError();
	if (!output.isEmpty()) logWarn("%1", output);

	// more mails?
	if (!queue_.isEmpty()) startProcess();
}

void Mailer::error(QProcess::ProcessError error)
{
	logFunctionTrace
	logWarn("Mailer process error: %1", (int)error);
}

void Mailer::doSend(const Mail & mail)
{
	if (!verifyThreadCall(&Mailer::doSend, mail)) return;

	if (sendmailPath_.isNull()) {
		logWarn("mailer not active, mail to %1 dropped", mail.to);
		return;
	}
	queue_ << mail;
	logInfo("queued new mail to %1 (queue size now: %2)", mail.to, queue_.size());

	if (process_->state() == QProcess::NotRunning) startProcess();
}

namespace {

QByteArray encodeAddress(const QString & address, QString & plain)
{
	static const QRegExp re("(.+)<(.+)>");
	if (re.indexIn(address) != -1) {
		plain = re.cap(2).trimmed();
		return cflib::util::encodeWord(re.cap(1).trimmed(), true) + " <" + plain.toLatin1() + ">";
	} else {
		plain = address.trimmed();
		return plain.toLatin1();
	}
}

}

void Mailer::startProcess()
{
	const Mail & mail = queue_.first();
	QString destAddress;
	const QByteArray raw = mail.raw(destAddress);
	logDebug("exec: %1 %2", sendmailPath_, destAddress);
	process_->start(sendmailPath_, QStringList() << destAddress);
	process_->write(raw);
	process_->closeWriteChannel();
}

QByteArray Mail::raw(QString & destAddress) const
{
	QByteArray rv;
	rv	<< "Content-type: text/plain; charset=utf-8\r\n"
		<< "Content-transfer-encoding: quoted-printable\r\n";
	QString addr;
	if (!from.isEmpty()) rv << "From: " << encodeAddress(from, addr) << "\r\n";
	rv	<< "To: " << encodeAddress(to, destAddress) << "\r\n"
		<< "Subject: " << cflib::util::encodeWord(subject, false) << "\r\n"
		<< "\r\n"
		<< cflib::util::encodeQuotedPrintable(text);
	return rv;
}


}}	// namespace

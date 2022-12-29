/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
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
	connect<void (QProcess::*)(int, QProcess::ExitStatus)>(
		process_, &QProcess::finished,
		this,       &Mailer::finished,
	Qt::DirectConnection);
	connect<void (QProcess::*)(QProcess::ProcessError)>(
		process_, &QProcess::errorOccurred,
		this,       &Mailer::error,
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
	QByteArray output;
	output += process_->readAllStandardOutput();
	output += process_->readAllStandardError();

	// check status
	if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
		if (!output.isEmpty()) logWarn("got output while sending email to %1: %2", queue_.first().to, output);
		else                   logInfo("email to %1 sent", queue_.first().to);
	} else {
		logWarn("could not send email to %1 (exitCode: %2, exitStatus: %3, output: %4)",
			queue_.first().to, exitCode, (int)exitStatus, output);
	}

	// more mails?
	queue_.removeFirst();
	if (!queue_.isEmpty()) execLater(new Functor0<Mailer>(this, &Mailer::startProcess));
}

void Mailer::error(QProcess::ProcessError error)
{
	logWarn("Mailer process error: %1", (int)error);
}

void Mailer::doSend(const Mail & mail)
{
	logFunctionTraceParam("new mail to: %1", mail.to);
	if (!verifyThreadCall(&Mailer::doSend, mail)) return;

	if (sendmailPath_.isNull()) {
		logWarn("mailer not active, mail to %1 dropped", mail.to);
		return;
	}

	queue_ << mail;
	if (queue_.size() == 1) startProcess();
}

namespace {

QByteArray encodeAddress(const QString & address, QString & plain)
{
	static const QRegExp re("(.+)<(.+)>");
	if (re.indexIn(address) != -1) {
		plain = re.cap(2).trimmed();
		return cflib::util::encodeWord(re.cap(1).trimmed(), true) + " <" + plain.toUtf8() + ">";
	} else {
		plain = address.trimmed();
		return plain.toUtf8();
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

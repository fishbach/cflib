/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
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

Mailer * Mailer::instance_ = nullptr;

Mailer::Mailer(bool isEnabled) :
    ThreadVerify("Mailer", ThreadVerify::Qt),
    process_(nullptr)
{
    if (instance_) logWarn("It makes no sense to have two Mailer instances!");
    instance_ = this;

    if (!isEnabled) {
        logInfo("emailing disabled");
        return;
    }

    QStringList paths = QProcessEnvironment::systemEnvironment().value("PATH").split(':');
    paths << "/usr/lib/" << "/usr/sbin/";
    for (QString & path : paths) {
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
    instance_ = nullptr;
}

void Mailer::send(const Mail & mail)
{
    if (!instance_) {
        logWarn("no Mailer instance available");
        return;
    }
    instance_->doSend(mail);
}

void Mailer::initThreadData()
{
    if (!verifyThreadCall(&Mailer::initThreadData)) return;
    logFunctionTrace

    process_ = new QProcess;
    connect<void (QProcess::*)(int, QProcess::ExitStatus)>(
        process_, &QProcess::finished,
        this,     &Mailer  ::finished,
    Qt::DirectConnection);
    connect<void (QProcess::*)(QProcess::ProcessError)>(
        process_, &QProcess::errorOccurred,
        this,     &Mailer  ::errorOccurred,
    Qt::DirectConnection);
}

void Mailer::deleteThreadData()
{
    if (process_ && process_->state() != QProcess::NotRunning) {
        logWarn("mail process still running");
        process_->kill();
    }
    delete process_;
    process_ = nullptr;
}

void Mailer::finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QByteArray output;
    output += process_->readAllStandardOutput();
    output += process_->readAllStandardError();

    // check status
    if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
        if (!output.isEmpty()) logWarn("got output while sending email to %1: %2", queue_.first().to, output);
        else                   logInfo("email from %1 to %2 sent", queue_.first().from, queue_.first().to);
    } else {
        logWarn("could not send email to %1 (exitCode: %2, exitStatus: %3, output: %4)",
            queue_.first().to, exitCode, (int)exitStatus, output);
    }

    // more mails?
    queue_.removeFirst();
    if (!queue_.isEmpty()) execLater([this]() { startProcess(); });
}

void Mailer::errorOccurred(QProcess::ProcessError error)
{
    logWarn("Mailer process error: %1", (int)error);
}

void Mailer::doSend(const Mail & mail)
{
    logFunctionTraceParam("new mail to: %1", mail.to);
    if (!verifyThreadCall(&Mailer::doSend, mail)) return;

    QString fromAddr;
    QString toAddr;
    if (!mail.isValid()) {
        logWarn("invalid mail: %1", mail.raw(fromAddr, toAddr));
        return;
    }

    if (sendmailPath_.isNull()) {
        logInfo("mailer not active, mail from %1 to %2 dropped", mail.from, mail.to);
        QTextStream(stdout)
            << "--------" << Qt::endl
            << mail.raw(fromAddr, toAddr) << Qt::endl
            << "--------" << Qt::endl;
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
    QString from;
    QString to;
    const QByteArray raw = mail.raw(from, to);
    logDebug("exec: %1 -f %2 %3", sendmailPath_, from, to);
    process_->start(sendmailPath_, {"-f", from, to});
    process_->write(raw);
    process_->closeWriteChannel();
}

bool Mail::isValid() const
{
    return !from.isEmpty() && !to.isEmpty();
}

QByteArray Mail::raw(QString & fromAddr, QString & toAddr) const
{
    QByteArray rv;
    rv  << "Content-type: text/plain; charset=utf-8\r\n"
        << "Content-transfer-encoding: quoted-printable\r\n"
        << "From: "    << encodeAddress(from, fromAddr)           << "\r\n"
        << "To: "      << encodeAddress(to, toAddr)               << "\r\n"
        << "Subject: " << cflib::util::encodeWord(subject, false) << "\r\n"
        << "\r\n"
        << cflib::util::encodeQuotedPrintable(text);
    return rv;
}


}}    // namespace

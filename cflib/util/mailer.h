/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/util/threadverify.h>

namespace cflib { namespace util {

class Mail
{
public:
    QString from;
    QString to;
    QString subject;
    QString text;

public:
    Mail() {}
    Mail(const QString & to, const QString & subject, const QString & text, const QString & from = QString()) :
        from(from), to(to), subject(subject), text(text) {}

    QByteArray raw(QString & destAddress) const;
};

class Mailer : public QObject, public ThreadVerify
{
    Q_OBJECT
public:
    Mailer();
    ~Mailer();

    static void send(const Mail & mail) { instance_->doSend(mail); }

protected:
    virtual void deleteThreadData();

private slots:
    void finished(int exitCode, QProcess::ExitStatus exitStatus);
    void errorOccurred(QProcess::ProcessError error);
    void startProcess();

private:
    void initThreadData();
    void doSend(const Mail & mail);

private:
    static Mailer * instance_;
    bool isFirstInstance_;

    QString sendmailPath_;
    QProcess * process_;
    QList<Mail> queue_;
};

}}    // namespace

/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
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
	void error(QProcess::ProcessError error);
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

}}	// namespace

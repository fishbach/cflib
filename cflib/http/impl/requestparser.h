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

#pragma once

#include <cflib/util/threadverify.h>

#include <QtNetwork>

namespace cflib { namespace http {

class RequestHandler;

namespace impl {

class RequestParser : public QObject, public util::ThreadVerify
{
	Q_OBJECT
public:
	RequestParser(qintptr sock, const QList<RequestHandler *> & handlers, util::ThreadVerify * tv);
	~RequestParser();

	void sendReply(int id, const QByteArray & reply);

	void detachRequest();
	QHostAddress getRemoteIP() const { return sock_.peerAddress(); }

private slots:
	void stateChanged(QAbstractSocket::SocketState state);
	void readyRead();

private:
	bool parseHeader();
	bool handleRequestLine(const QByteArray & line);
	void writeReply(const QByteArray & reply);

private:
	QTcpSocket sock_;
	const QList<RequestHandler *> & handlers_;
	const int id_;

	QByteArray header_;

	qint64 contentLength_;
	QHash<QByteArray, QByteArray> headerFields_;
	QByteArray method_;
	QUrl url_;
	QByteArray body_;

	int requestCount_;
	int nextReplyId_;
	QMap<int, QByteArray> replies_;

	int attachedRequests_;
	bool socketClosed_;
};

}}}	// namespace

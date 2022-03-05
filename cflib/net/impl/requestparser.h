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

#include <cflib/net/tcpconn.h>
#include <cflib/util/threadverify.h>

namespace cflib { namespace net {

class PassThroughHandler;
class RequestHandler;

namespace impl {

class HttpThread;

class RequestParser : public util::ThreadVerify, public TCPConn
{
public:
	RequestParser(TCPConnData * data,
		const QList<RequestHandler *> & handlers, HttpThread * thread);
	~RequestParser();

	void sendReply(int id, const QByteArray & reply);

	void detachRequest();
	void setPassThroughHandler(PassThroughHandler * hdl);
	QByteArray readPassThrough(bool & isLast);
	TCPConnData * detach();

protected:
	virtual void newBytesAvailable();
	virtual void closed(CloseType type);

private:
	void parseRequest();
	bool parseHeader();
	bool handleRequestLine(const QByteArray & line);
	void writeReply(const QByteArray & reply);

private:
	const QList<RequestHandler *> & handlers_;
	HttpThread * thread_;
	const int id_;

	QByteArray header_;

	qint64 contentLength_;
	QMap<QByteArray, QByteArray> headerFields_;
	int method_;
	QByteArray uri_;
	QByteArray body_;

	int requestCount_;
	int nextReplyId_;
	QMap<int, QByteArray> replies_;

	int attachedRequests_;
	bool detached_;
	bool passThrough_;
	PassThroughHandler * passThroughHandler_;
};

}}}	// namespace

/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
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

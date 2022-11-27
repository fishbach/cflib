/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/net/requesthandler.h>
#include <cflib/net/tcpconn.h>
#include <cflib/util/threadverify.h>

namespace cflib { namespace util { class EVTimer; }}

namespace cflib { namespace net {

class WebSocketService : public RequestHandler, public util::ThreadVerify
{
public:
	WebSocketService(const QString & path, const QRegularExpression & allowedOrigin = QRegularExpression(),
		uint connectionTimeoutSec = 0);
	~WebSocketService();

protected:
	void send(uint connId, const QByteArray & data, bool isBinary);
	void close(uint connId, TCPConn::CloseType type = TCPConn::ReadWriteClosed);
	QByteArray getRemoteIP(uint connId);
	void continueRead(uint connId);

	virtual void newConnection(uint connId);
	virtual void newMsg(uint connId, const QByteArray & data, bool isBinary, bool & stopRead) = 0;
	virtual void closed(uint connId, TCPConn::CloseType type);

	virtual void handleRequest(const Request & request);

private:
	void addConnection(TCPConnData * connData, const QByteArray & wsKey, bool deflate);
	void startTimer();
	void checkTimeout();

private:
	const QString path_;
	const QRegularExpression allowedOrigin_;
	const uint connectionTimeoutSec_;
	class WSConnHandler;
	QHash<uint, WSConnHandler *> connections_;
	uint lastConnId_;
	util::EVTimer * timer_;
};

}}	// namespace

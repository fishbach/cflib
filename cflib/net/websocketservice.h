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

#include <cflib/net/requesthandler.h>
#include <cflib/net/tcpconn.h>
#include <cflib/util/threadverify.h>

namespace cflib { namespace util { class EVTimer; }}

namespace cflib { namespace net {

class ApiServer;

class WebSocketService : public RequestHandler, public util::ThreadVerify
{
public:
	WebSocketService(const QString & path, uint connectionTimeoutSec = 0);
	~WebSocketService();

protected:
	void send(uint connId, const QByteArray & data, bool isBinary);
	void close(uint connId, TCPConn::CloseType type = TCPConn::ReadWriteClosed);
	QByteArray getRemoteIP(uint connId);

	virtual void newConnection(uint connId);
	virtual void newMsg(uint connId, const QByteArray & data, bool isBinary) = 0;
	virtual void closed(uint connId, TCPConn::CloseType type);

	virtual void handleRequest(const Request & request);

private:
	void addConnection(const Request & request);
	void startTimer();
	void checkTimeout();

private:
	const QString path_;
	const uint connectionTimeoutSec_;
	class WSConnHandler;
	QHash<uint, WSConnHandler *> connections_;
	uint lastConnId_;
	util::EVTimer * timer_;
};

}}	// namespace

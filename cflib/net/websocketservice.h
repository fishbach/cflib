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
#include <cflib/util/threadverify.h>

namespace cflib { namespace net {

class ApiServer;

class WebSocketService : public QObject, public RequestHandler, public util::ThreadVerify
{
	Q_OBJECT
public:
	WebSocketService(const QString & path);
	~WebSocketService();

	void send(uint clientId, const QByteArray & data, bool isBinary);
	void sendAll(const QByteArray & data, bool isBinary);
	void close(uint clientId);

signals:
	void newClient(uint clientId, bool & abort);
	void newMsg(uint clientId, const QByteArray & data, bool isBinary);
	void closed(uint clientId);

	void getClientId(const QByteArray & clIdData, uint & clId);

protected:
	virtual void handleRequest(const Request & request);

private:
	const QString path_;
	class WSConnHandler;
	QMultiHash<uint, WSConnHandler *> clients_;
	QSet<WSConnHandler *> all_;
};

}}	// namespace

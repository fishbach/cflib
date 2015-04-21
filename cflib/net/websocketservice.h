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

class WebSocketService : public RequestHandler
{
public:
	WebSocketService(const ApiServer & apiServer, const QString & path);

	void send(uint clientId, const QByteArray & data, bool isBinary);
	void sendAll(const QByteArray & data, bool isBinary);
	void close(uint clientId);

protected:
	virtual void newMsg(uint clientId, const QByteArray & data, bool isBinary) = 0;
	virtual bool newClient(uint clientId);
	virtual void closed(uint clientId);

	virtual void handleRequest(const Request & request);

private:
	const ApiServer & apiServer_;
	const QString path_;
	class WSConnHandler;
	QMultiHash<uint, WSConnHandler *> clients_;
	QSet<WSConnHandler *> all_;
};

}}	// namespace

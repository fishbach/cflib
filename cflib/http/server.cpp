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

#include "server.h"

#include <cflib/http/impl/requestparser.h>
#include <cflib/util/log.h>
#include <cflib/util/tcpserver.h>

USE_LOG(LogCat::Http)

namespace cflib { namespace http {

class Server::Impl : public util::ThreadVerify, public util::TCPServer
{
public:
	Impl() :
		util::ThreadVerify("HTTP-Server", true)
	{
	}

	~Impl()
	{
		stopVerifyThread();
	}

	bool start(quint16 port, const QByteArray & address)
	{
		SyncedThreadCall<bool> stc(this);
		if (!stc.verify(&Impl::start, port, address)) return stc.retval();

		if (handlers_.isEmpty()) {
			logWarn("no handlers registered");
			return false;
		}

		if (util::TCPServer::start(port, address)) {
			logInfo("HTTP-Server started on %1:%2", address, port);
			return true;
		} else {
			logWarn("HTTP-Server cannot listen on %1:%2", address, port);
			return false;
		}
	}

	void registerHandler(RequestHandler * handler)
	{
		if (!verifyThreadCall(&Impl::registerHandler, handler)) return;

		handlers_ << handler;
	}

protected:
	virtual void newConnection(const ConnInitializer * init)
    {
		if (!verifyThreadCall(&Impl::newConnection, init)) return;

		new impl::RequestParser(init, handlers_, this);
    }

private:
	QList<RequestHandler *> handlers_;
};

Server::Server() :
	impl_(new Impl())
{
}

Server::~Server()
{
	delete impl_;
}

bool Server::start(quint16 port, const QByteArray & address)
{
	return impl_->start(port, address);
}

void Server::registerHandler(RequestHandler * handler)
{
	impl_->registerHandler(handler);
}

}}	// namespace

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

USE_LOG(LogCat::Http)

namespace cflib { namespace http {

class Server::Impl : public util::ThreadVerify, public QTcpServer
{
public:
	Impl() :
		util::ThreadVerify("HTTP-Server")
	{
		moveToThread(threadObject()->thread());
		setParent(threadObject());
	}

	bool start(quint16 port, const QHostAddress & address)
	{
		SyncedThreadCall<bool> stc(this);
		if (!stc.verify(&Impl::start, port, address)) return stc.retval();

		if (handlers_.isEmpty()) {
			logWarn("no handlers registered");
			return false;
		}

		if (listen(address, port)) {
			logInfo("HTTP-Server started on %1:%2", address.toString(), port);
			return true;
		} else {
			logWarn("HTTP-Server cannot listen on %1:%2", address.toString(), port);
			return false;
		}
	}

	void registerHandler(RequestHandler * handler)
	{
		if (!verifyThreadCall(&Impl::registerHandler, handler)) return;

		handlers_ << handler;
	}

protected:
    virtual void incomingConnection(qintptr handle)
    {
		new impl::RequestParser(handle, handlers_, this);
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
	impl_->stopVerifyThread();
	// delete is done by the parent (thread object)
}

bool Server::start(quint16 port, const QHostAddress & address)
{
	return impl_->start(port, address);
}

void Server::registerHandler(RequestHandler * handler)
{
	impl_->registerHandler(handler);
}

}}	// namespace

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

#include "forwardserver.h"

#include <cflib/http/request.h>
#include <cflib/util/log.h>
#include <cflib/util/tcpserver.h>

using namespace cflib::util;

USE_LOG(LogCat::Http)

namespace cflib { namespace http {

namespace {

class TCPForwarder;

class TCPReader : public TCPConn
{
public:
	TCPReader(const TCPConnInitializer * init, TCPForwarder * forwarder) :
		TCPConn(init), forwarder_(forwarder)
	{
		startWatcher();
	}

	~TCPReader()
	{
		logFunctionTrace
	}

	void detach()
	{
		forwarder_ = 0;
	}

protected:
	virtual void newBytesAvailable();
	virtual void closed();

private:
	TCPForwarder * forwarder_;
};

class TCPForwarder : public TCPConn
{
public:
	TCPForwarder(const TCPConnInitializer * init, const Request & request) :
		TCPConn(init),
		reader_(0)
	{
		logFunctionTrace

		QByteArray requestData = request.getHeader();
		requestData += "\r\n\r\n";
		requestData += request.getBody();
		write(requestData);
		const TCPConnInitializer * oldInit = request.detachFromSocket();
		if (!oldInit) {
			logWarn("could not detach from socket");
			destroy();
		} else {
			reader_ = new TCPReader(oldInit, this);
			startWatcher();
		}
	}

	~TCPForwarder()
	{
		logFunctionTrace
	}

	void detach()
	{
		reader_ = 0;
	}

protected:
	virtual void newBytesAvailable()
	{
		logFunctionTrace
		const QByteArray data = read();
		if (reader_) reader_->write(data);
		startWatcher();
	}

	virtual void closed()
	{
		logFunctionTrace
		if (reader_) {
			reader_->detach();
			reader_->closeNicely();
		}
		destroy();
	}

private:
	TCPReader * reader_;
};

void TCPReader::newBytesAvailable()
{
	logFunctionTrace
	const QByteArray data = read();
	if (forwarder_) forwarder_->write(data);
	startWatcher();
}

void TCPReader::closed()
{
	logFunctionTrace
	if (forwarder_) {
		forwarder_->detach();
		forwarder_->closeNicely();
	}
	destroy();
}

}

void ForwardServer::handleRequest(const Request & request)
{
	const QByteArray hostname = request.getHeaderFields().value("host");
	if (okHosts_.contains(hostname)) return;
	TCPServer * tcpServ = TCPServer::instance();
	if (tcpServ && forwardPort_ > 0) {
		const TCPConnInitializer * ci = tcpServ->openConnection(forwardIP_, forwardPort_);
		if (!ci) {
			logInfo("cannot open connection to %1:%2", forwardIP_, forwardPort_);
			request.sendNotFound();
			return;
		}
		new TCPForwarder(ci, request);
	} else {
		logWarn("cannot forward request");
	}
}

}}	// namespace

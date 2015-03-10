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

#include "redirectserver.h"

#include <cflib/net/request.h>
#include <cflib/net/tcpserver.h>
#include <cflib/util/log.h>
#include <cflib/util/util.h>

using namespace cflib::util;

USE_LOG(LogCat::Http)

namespace cflib { namespace net {

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

		QByteArray requestData = request.getRawHeader();
		requestData += "\r\n\r\n";
		requestData += request.getBody();
		write(requestData);
		const TCPConnInitializer * oldInit = request.detachFromSocket();
		if (!oldInit) {
			logWarn("could not detach from socket");
			deleteNext(this);
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
		deleteNext(this);
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
	deleteNext(this);
}

}

void RedirectServer::addValid(const QRegularExpression & test)
{
	entries_ << Entry(test);
}

void RedirectServer::addRedirectIf(const QRegularExpression & test, const QByteArray & destUrl)
{
	entries_ << Entry(false, test, destUrl);
}

void RedirectServer::addRedirectIf(const QRegularExpression & test, DestUrlReFunc destUrlReFunc)
{
	entries_ << Entry(false, test, destUrlReFunc);
}

void RedirectServer::addRedirectIfNot(const QRegularExpression & test, const QByteArray & destUrl)
{
	entries_ << Entry(true, test, destUrl);
}

void RedirectServer::addDefaultRedirect(const QByteArray & destUrl)
{
	entries_ << Entry(destUrl);
}

void RedirectServer::addDefaultRedirect(DestUrlFunc destUrlFunc)
{
	entries_ << Entry(destUrlFunc);
}

void RedirectServer::addForwardIf(const QRegularExpression & test, const QByteArray & ip, quint16 port)
{
	entries_ << Entry(false, test, qMakePair(ip, port));
}

void RedirectServer::addForwardIf(const QRegularExpression & test, DestHostReFunc destHostReFunc)
{
	entries_ << Entry(false, test, destHostReFunc);
}

void RedirectServer::addForwardIfNot(const QRegularExpression & test, const QByteArray & ip, quint16 port)
{
	entries_ << Entry(true, test, qMakePair(ip, port));
}

void RedirectServer::addDefaultForward(const QByteArray & ip, quint16 port)
{
	entries_ << Entry(qMakePair(ip, port));
}

void RedirectServer::addDefaultForward(DestHostFunc destHostFunc)
{
	entries_ << Entry(destHostFunc);
}

void RedirectServer::handleRequest(const cflib::net::Request & request)
{
	QString url = request.getHostname();
	url += request.getUri();
	QListIterator<Entry> it(entries_);
	while (it.hasNext()) {
		const Entry & entry = it.next();

		QRegularExpressionMatch match;
		if (!entry.isDefault) {
			match = entry.test.match(url);
			if (match.hasMatch() == entry.invert) continue;
			if (entry.isValid) return;
		}

		if (entry.isRedirect) {
			if (entry.isDefault) {
				request.sendRedirect(entry.destUrlFunc ? entry.destUrlFunc(request) : entry.destUrl);
			} else {
				request.sendRedirect(entry.destUrlReFunc ? entry.destUrlReFunc(request, match) : entry.destUrl);
			}
			return;
		}

		TCPServer * tcpServ = request.tcpServer();
		if (!tcpServ) {
			logWarn("cannot forward request");
			return;
		}

		DestHost destHost;
		if (entry.isDefault) {
			destHost = entry.destHostFunc ? entry.destHostFunc(request) : entry.destHost;
		} else {
			destHost = entry.destHostReFunc ? entry.destHostReFunc(request, match) : entry.destHost;
		}
		const TCPConnInitializer * ci = tcpServ->openConnection(destHost.first, destHost.second);
		if (!ci) {
			logInfo("cannot open connection to %1:%2", destHost.first, destHost.second);
			request.sendNotFound();
			return;
		}
		new TCPForwarder(ci, request);
		return;
	}
}

}}	// namespace

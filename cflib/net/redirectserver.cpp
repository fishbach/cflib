/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "redirectserver.h"

#include <cflib/net/request.h>
#include <cflib/net/tcpconn.h>
#include <cflib/net/tcpmanager.h>
#include <cflib/util/log.h>
#include <cflib/util/threadverify.h>
#include <cflib/util/util.h>

using namespace cflib::util;

USE_LOG(LogCat::Http)

namespace cflib { namespace net {

namespace {

class TCPForwarder;

class TCPReader : public TCPConn, public ThreadVerify
{
public:
	TCPReader(TCPConnData * data, TCPForwarder * forwarder) :
		TCPConn(data),
		ThreadVerify(manager().networkThread()),
		forwarder_(forwarder)
	{
		startReadWatcher();
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
	virtual void closed(CloseType type);

private:
	TCPForwarder * forwarder_;
};

// Forwarder holds outgoing connection
class TCPForwarder : public TCPConn, public ThreadVerify
{
public:
	TCPForwarder(TCPConnData * data, const Request & request) :
		TCPConn(data),
		ThreadVerify(manager().networkThread()),
		reader_(0)
	{
		logFunctionTrace

		// write request
		QByteArray requestData = request.getRawHeader();
		requestData += "\r\n\r\n";
		requestData += request.getBody();
		write(requestData);

		// take incoming tcp connection from request
		TCPConnData * oldData = request.detach();
		if (!oldData) {
			logWarn("could not detach from socket");
			deleteNext(this);
			return;
		}

		// wait for more requests from client
		reader_ = new TCPReader(oldData, this);

		// wait for reply from outgoing connection
		startReadWatcher();
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
		// we might get called by TLS-Thread here
		if (!verifyThreadCall(&TCPForwarder::newBytesAvailable)) return;

		const QByteArray data = read();
		logTrace("read %1 bytes from outgoing connection", data.size());
		if (reader_) reader_->write(data);
		startReadWatcher();
	}

	virtual void closed(CloseType type)
	{
		// we might get called by TLS-Thread here
		if (!verifyThreadCall(&TCPForwarder::closed, type)) return;

		logTrace("outgoing connection closed with %1", (int)type);
		if (type & ReadClosed) {
			if (reader_) {
				reader_->detach();
				reader_->close(ReadWriteClosed, true);
			}
			deleteNext(this);
		}
	}

private:
	TCPReader * reader_;
};

void TCPReader::newBytesAvailable()
{
	// we might get called by TLS-Thread here
	if (!verifyThreadCall(&TCPReader::newBytesAvailable)) return;

	const QByteArray data = read();
	logTrace("read %1 bytes from incoming connection", data.size());
	if (forwarder_) forwarder_->write(data);
	startReadWatcher();
}

void TCPReader::closed(CloseType type)
{
	// we might get called by TLS-Thread here
	if (!verifyThreadCall(&TCPReader::closed, type)) return;

	logTrace("incoming connection closed with %1", (int)type);
	if (type & WriteClosed) {
		if (forwarder_) {
			forwarder_->detach();
			forwarder_->close(ReadWriteClosed);
		}
		deleteNext(this);
	} else if (forwarder_) forwarder_->close(WriteClosed);
}

}

void RedirectServer::addValid(const QRegularExpression & test)
{
	entries_ << Entry(test);
}

void RedirectServer::addRedirectIf(const QRegularExpression & test, const char * destUrl)
{
	entries_ << Entry(false, test, QByteArray(destUrl));
}

void RedirectServer::addRedirectIf(const QRegularExpression & test, const QByteArray & destUrl)
{
	entries_ << Entry(false, test, destUrl);
}

void RedirectServer::addRedirectIf(const QRegularExpression & test, DestUrlReFunc destUrlReFunc)
{
	entries_ << Entry(false, test, destUrlReFunc);
}

void RedirectServer::addRedirectIfNot(const QRegularExpression & test, const char * destUrl)
{
	entries_ << Entry(true, test, QByteArray(destUrl));
}

void RedirectServer::addRedirectIfNot(const QRegularExpression & test, const QByteArray & destUrl)
{
	entries_ << Entry(true, test, destUrl);
}

void RedirectServer::addDefaultRedirect(const char * destUrl)
{
	entries_ << Entry(QByteArray(destUrl));
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

		// check for match
		QRegularExpressionMatch match;
		if (!entry.isDefault) {
			match = entry.test.match(url);
			if (match.hasMatch() == entry.invert) continue;
			if (entry.isValid) return;
		}

		// handle redirect
		if (entry.isRedirect) {
			if (entry.isDefault) {
				request.sendRedirect(entry.destUrlFunc ? entry.destUrlFunc(request) : entry.destUrl);
			} else {
				request.sendRedirect(entry.destUrlReFunc ? entry.destUrlReFunc(request, match) : entry.destUrl);
			}
			return;
		}

		// handle forward (pass through)
		TCPManager * tcpManager = request.tcpManager();
		if (!tcpManager) {
			logWarn("cannot forward request");
			return;
		}

		// open destination connection
		DestHost destHost;
		if (entry.isDefault) {
			destHost = entry.destHostFunc ? entry.destHostFunc(request) : entry.destHost;
		} else {
			destHost = entry.destHostReFunc ? entry.destHostReFunc(request, match) : entry.destHost;
		}
		TCPConnData * ci = tcpManager->openConnection(destHost.first, destHost.second);
		if (!ci) {
			logInfo("cannot open connection to %1:%2", destHost.first, destHost.second);
			request.sendNotFound();
			return;
		}

		// start tcp forwarding
		new TCPForwarder(ci, request);
		return;
	}
}

}}	// namespace

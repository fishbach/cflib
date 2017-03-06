/* Copyright (C) 2013-2017 Christian Fischbach <cf@cflib.de>
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

#include "request.h"

#include <cflib/net/requesthandler.h>
#include <cflib/net/impl/requestparser.h>
#include <cflib/util/log.h>
#include <cflib/util/util.h>

USE_LOG(LogCat::Http)

namespace cflib { namespace net {

class Request::Shared
{
public:
	Shared(int connId, int requestId,
		const QByteArray & header,
		const Request::KeyVal & headerFields, Request::Method method, const QByteArray & uri,
		const QByteArray & body, const QList<RequestHandler *> & handlers, bool passThrough,
		impl::RequestParser * parser)
	:
		ref(1),
		connId(connId),
		requestId(requestId),
		header(header),
		headerFields(headerFields), method(method), uri(uri), body(body),
		handlers(handlers),
		parser(parser),
		replySent(parser == 0),
		passThrough(passThrough),
		detached(false)
	{
		if (headerFields.contains("x-remote-ip")) {
			remoteIP = headerFields.value("x-remote-ip");
		} else if (parser) {
			remoteIP = parser->peerIP();
		}
		watch.start();
		id << QByteArray::number(connId) << '-' << QByteArray::number(requestId);
		logDebug("new request %1 (body len: %2)", id, headerFields.value("content-length"));
	}

	~Shared()
	{
		const int msec = watch.elapsed();
		if (detached) {
			logDebug("request %1 detached", id);
		} else if (!replySent) {
			sendNotFound();
			logDebug("request %1 finished with 404 (msec: %2)", id, msec);
		} else {
			logDebug("request %1 finished successfully (msec: %2)", id, msec);
		}

		if (parser) parser->detachRequest();
	}

	QAtomicInt ref;
	int connId;
	int requestId;
	QByteArray id;
	QByteArray header;
	Request::KeyVal headerFields;
	Request::Method method;
	QByteArray uri;
	QByteArray body;
	QList<RequestHandler *> handlers;
	impl::RequestParser * parser;
	QTime watch;
	bool replySent;
	QByteArray remoteIP;
	QList<QByteArray> sendHeaderLines;
	bool passThrough;
	bool detached;

public:
	QByteArray getRequestField(const QByteArray & key)
	{
		const QByteArray search = key.toLower();
		QMapIterator<QByteArray, QByteArray> it(headerFields);
		while (it.hasNext()) {
			it.next();
			if (it.key().toLower() == search) return it.value();
		}
		return QByteArray();
	}

	void sendReply(QByteArray header, QByteArray body, bool compression)
	{
		if (replySent) {
			logWarn("tried to send two replies for request %1", id);
			return;
		}
		replySent = true;

		// compression
		if (compression && method != Request::HEAD && body.size() > 256 &&
			getRequestField("Accept-Encoding").indexOf("gzip") != -1)
		{
			header += "Content-Encoding: gzip\r\n";
			cflib::util::gzip(body, 1);
		}

		QListIterator<QByteArray> it(sendHeaderLines);
		while (it.hasNext()) header << it.next() << "\r\n";
		if (method != Request::HEAD) {
			header
				<< "Content-Length: " << QByteArray::number(body.size()) << "\r\n"
				<< "\r\n"
				<< body;
		} else {
			header += "\r\n";
		}

		parser->sendReply(requestId, header);
	}

	void sendNotFound()
	{
		sendReply(
			"HTTP/1.1 404 Not Found\r\n"
			<< defaultHeaders() <<
			"Content-Type: text/html; charset=utf-8\r\n",

			"<html>\r\n"
			"<head><title>404 - Not Found</title></head>\r\n"
			"<body>\r\n"
			"<h1>404 - Not Found</h1>\r\n"
			"</body>\r\n"
			"</html>\r\n",
			false);
	}

	inline QByteArray defaultHeaders()
	{
		return
			"Date: " << cflib::util::dateTimeForHTTP(QDateTime::currentDateTime()) << "\r\n"
			"Connection: keep-alive\r\n"
			"Server: cflib/0.9\r\n";
	}

};

Request::Request() :
	d(new Shared(0, 0, QByteArray(), KeyVal(), NONE, QByteArray(), QByteArray(), QList<RequestHandler *>(), false, 0))
{
}

Request::Request(int connId, int requestId,
	const QByteArray & header,
	const KeyVal & headerFields, Method method, const QByteArray & uri,
	const QByteArray & body, const QList<RequestHandler *> & handlers, bool passThrough,
	impl::RequestParser * parser)
:
	d(new Shared(connId, requestId, header, headerFields, method, uri, body, handlers, passThrough, parser))
{
}

Request::~Request()
{
	while (!d->ref.deref()) {
		if (d->replySent || d->handlers.isEmpty()) {
			logTrace("request deleted");
			delete d;
			return;
		}
		d->ref.ref();
		callNextHandler();
	}
}

Request::Request(const Request & other) :
	d(other.d)
{
	d->ref.ref();
}

Request & Request::operator=(const Request & other)
{
	qAtomicAssign(d, other.d);
	return *this;
}

Request::Id Request::getId() const
{
	return qMakePair(d->connId, d->requestId);
}

bool Request::replySent() const
{
	return d->replySent;
}

QByteArray Request::getRawHeader() const
{
	return d->header;
}

QByteArray Request::getHeader(const QByteArray & name) const
{
	return d->headerFields.value(name);
}

QByteArray Request::getHostname() const
{
	return d->headerFields.value("host");
}

Request::KeyVal Request::getHeaderFields() const
{
	return d->headerFields;
}

Request::Method Request::getMethod() const
{
	return d->method;
}

QByteArray Request::getMethodName() const
{
	switch (d->method) {
		case NONE: return "-";
		case GET:  return "GET";
		case POST: return "POST";
		case HEAD: return "HEAD";
	}
	return QByteArray();
}

QByteArray Request::getUri() const
{
	return d->uri;
}

QByteArray Request::getBody() const
{
	return d->body;
}

QByteArray Request::getRemoteIP() const
{
	return d->remoteIP;
}

void Request::sendNotFound() const
{
	d->sendNotFound();
}

void Request::sendRedirect(const QByteArray & url) const
{
	d->sendReply(
		"HTTP/1.1 307 Temporary Redirect\r\n"
		"Location: " << url << "\r\n"
		<< d->defaultHeaders() <<
		"Content-Type: text/html; charset=utf-8\r\n",

		"<html>\r\n"
		"<head><title>307 - Temporary Redirect</title></head>\r\n"
		"<body>\r\n"
		"<h1>307 - Temporary Redirect</h1>\r\n"
		"</body>\r\n"
		"</html>\r\n",
		false);
}

void Request::sendReply(const QByteArray & reply, const QByteArray & contentType, bool compression) const
{
	d->sendReply(
		"HTTP/1.1 200 OK\r\n"
		<< d->defaultHeaders() <<
		"Content-Type: " << contentType << "\r\n",
		reply, compression);
}

void Request::sendText(const QString & reply, const QByteArray & contentType, bool compression) const
{
	sendReply(reply.toUtf8(), contentType + "; charset=utf-8", compression);
}

void Request::sendRaw(const QByteArray & header, const QByteArray & body, bool compression) const
{
	d->sendReply(header, body, compression);
}

void Request::addHeaderLine(const QByteArray & line) const
{
	d->sendHeaderLines << line;
}

QByteArray Request::defaultHeaders() const
{
	return d->defaultHeaders();
}

bool Request::isPassThrough() const
{
	return d->passThrough;
}

void Request::setPassThroughHandler(PassThroughHandler * hdl) const
{
	if (d->parser) d->parser->setPassThroughHandler(hdl);
}

QByteArray Request::readPassThrough(bool & isLast) const
{
	if (!d->parser) return QByteArray();
	return d->parser->readPassThrough(isLast);
}

void Request::startWatcher() const
{
	if (d->parser) d->parser->startReadWatcher();
}

void Request::abort() const
{
	if (d->parser) d->parser->close(TCPConn::HardClosed);
}

TCPConnData * Request::detach() const
{
	if (!d->parser) return 0;
	d->replySent = true;
	d->detached = true;
	return d->parser->detach();
}

TCPManager * Request::tcpManager() const
{
	if (!d->parser) return 0;
	return &d->parser->manager();
}

void Request::callNextHandler() const
{
	d->handlers.takeFirst()->handleRequest(*this);
}

}}	// namespace

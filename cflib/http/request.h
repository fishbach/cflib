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

#include <QtCore>
#include <QtNetwork>

namespace cflib { namespace http {

class RequestHandler;
namespace impl { class RequestParser; }

class PassThroughHandler
{
public:
	virtual void morePassThroughData(int connId, int requestId) = 0;
};

class Request
{
public:
	typedef QHash<QByteArray, QByteArray> KeyVal;

public:
	Request();
	Request(int connId, int requestId,
		const KeyVal & headerFields, const QByteArray & method, const QUrl & url, const QByteArray & body,
		const QList<RequestHandler *> & handlers,
		bool passThrough,
		impl::RequestParser * parser);

	// implicit sharing
	~Request();
	Request(const Request & other);
	Request & operator=(const Request & other);

	bool replySent() const;

	KeyVal getHeaderFields() const;
	bool isGET() const;
	bool isPOST() const { return !isGET(); }
	QUrl getUrl() const;
	QByteArray getBody() const;
	QHostAddress getRemoteIP() const;

	void sendNotFound() const;
	void sendRedirect(const QByteArray & url) const;
	void sendReply(const QByteArray & reply, const QByteArray & contentType, bool compression = true) const;
	void sendText(const QString & reply, const QByteArray & contentType = "text/html", bool compression = true) const;
	void addHeaderLine(const QByteArray & line) const;

	bool isPassThrough() const;
	void setPassThroughHandler(PassThroughHandler * hdl) const;
	QByteArray readPassThrough(bool & isLast) const;

private:
	void callNextHandler() const;

private:
	class Shared;
	Shared * d;
	friend class impl::RequestParser;
};

}}	// namespace

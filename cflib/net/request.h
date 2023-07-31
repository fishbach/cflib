/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <QtCore>

namespace cflib { namespace net {

class PassThroughHandler;
class RequestHandler;
class TCPConnData;
class TCPManager;
namespace impl { class RequestParser; }

class Request
{
public:
	typedef QPair<int, int> Id;
	typedef QMap<QByteArray, QByteArray> KeyVal;
	enum Method {
		NONE = 0,
		GET,
		POST,
		HEAD
	};

public:
	Request();
	Request(int connId, int requestId,
		const QByteArray & header,
		const KeyVal & headerFields, Method method, const QByteArray & uri,
		const QByteArray & body, const QList<RequestHandler *> & handlers, bool passThrough,
		impl::RequestParser * parser);

	// implicit sharing
	~Request();
	Request(const Request & other);
	Request & operator=(const Request & other);

	Id getId() const;
	bool replySent() const;

	QByteArray getRawHeader() const;
	QByteArray getHeader(const QByteArray & name) const;
	QByteArray getHostname() const;
	KeyVal getHeaderFields() const;
	Method getMethod() const;
	QByteArray getMethodName() const;
	inline bool isGET()  const { return getMethod() == GET; }
	inline bool isPOST() const { return getMethod() == POST; }
	inline bool isHEAD() const { return getMethod() == HEAD; }
	QByteArray getUri() const;
	QByteArray getBody() const;
	QByteArray getRemoteIP() const;

	void sendNotFound() const;
	void sendRedirect(const QByteArray & url) const;
	void sendReply(const QByteArray & reply, const QByteArray & contentType, bool compression = true) const;
	void sendText(const QString & reply, const QByteArray & contentType = "text/html", bool compression = true) const;
	void sendRaw(const QByteArray & header, const QByteArray & body, bool compression) const;
	void addHeaderLine(const QByteArray & line) const;
	QByteArray defaultHeaders() const;

	bool isPassThrough() const;
	void setPassThroughHandler(PassThroughHandler * hdl) const;
	QByteArray readPassThrough(bool & isLast) const;
	void startWatcher() const;
	void abort() const;

	TCPConnData * detach() const;
	TCPManager * tcpManager() const;

private:
	void callNextHandler() const;

private:
	class Shared;
	Shared * d;
	friend class impl::RequestParser;
};

class PassThroughHandler
{
public:
	virtual void morePassThroughData() = 0;
};

}}	// namespace

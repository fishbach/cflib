/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <QtCore>

namespace cflib { namespace net {

class TCPManager;

class HttpClient
{
	Q_DISABLE_COPY(HttpClient)
public:
	HttpClient(TCPManager & mgr, bool keepAlive = true);
	~HttpClient();

	// TODO: getaddrinfo -> dns resolve
	void get(const QByteArray & ip, quint16 port, const QByteArray & url);

protected:
	virtual void reply(const QByteArray & raw) = 0;

private:
	TCPManager & mgr_;
	bool keepAlive_;
	class HttpConn;
	HttpConn * conn_;
};

}}	// namespace

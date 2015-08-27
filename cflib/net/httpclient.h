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

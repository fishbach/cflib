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

namespace cflib { namespace crypt { class TLSCredentials; }}

namespace cflib { namespace http {

class RequestHandler;

class Server
{
	Q_DISABLE_COPY(Server)
public:
	Server(uint threadCount = 2);
	Server(crypt::TLSCredentials & credentials, uint threadCount = 2);
	~Server();

	bool start(int listenSocket);
	bool start(quint16 port, const QByteArray & address = "127.0.0.1");
	bool stop();
	bool isRunning() const;

	void registerHandler(RequestHandler * handler);

private:
	class Impl;
	Impl * impl_;
};

}}	// namespace

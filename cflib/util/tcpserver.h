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

#include <cflib/libev/libev.h>

#include <QtCore>

namespace cflib { namespace util {

class TCPServer
{
	Q_DISABLE_COPY(TCPServer)
public:
	TCPServer();
	virtual ~TCPServer();

	bool start(quint16 port, const QByteArray & ip);

protected:
	virtual void newConnection(int socket, const QByteArray & peerIP, quint16 peerPort) = 0;

private:
	class Impl;
	Impl * impl_;
};

class TCPConn
{
	Q_DISABLE_COPY(TCPConn)
public:
	TCPConn(TCPServer * server, int socket);
	virtual ~TCPConn();

	QByteArray read();
	void write(const QByteArray & data);
	void close();	// int shutdown(int socket, int how);

protected:
	virtual void newBytesAvailable() = 0;
	virtual void closedByPeer() = 0;

private:
	const int socket_;
	QByteArray writeBuf_;
	ev_io readWatcher_;
	ev_io writeWatcher_;
};

}}	// namespace

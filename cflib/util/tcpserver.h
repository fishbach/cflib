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

struct ev_io;
struct ev_loop;

namespace cflib { namespace util {

class TCPServer
{
	Q_DISABLE_COPY(TCPServer)
public:
	class ConnInitializer;
	class Impl;
public:
	TCPServer();
	virtual ~TCPServer();

	bool start(quint16 port, const QByteArray & ip = "0.0.0.0");

protected:
	virtual void newConnection(const ConnInitializer * init) = 0;

private:
	Impl * impl_;
};

class TCPConn
{
	Q_DISABLE_COPY(TCPConn)
public:
	TCPConn(const TCPServer::ConnInitializer * init);
	virtual ~TCPConn();

	QByteArray read();
	void startReadWatcher();
	void write(const QByteArray & data);
	void close();

	QByteArray peerIP() const { return peerIP_; }
	quint16 peerPort() const { return peerPort_; }

protected:
	virtual void newBytesAvailable() = 0;
	virtual void closed() = 0;

private:
	static void readable (ev_loop * loop, ev_io * w, int revents);
	static void writeable(ev_loop * loop, ev_io * w, int revents);

private:
	TCPServer::Impl & impl_;
	int socket_;
	const QByteArray peerIP_;
	const quint16 peerPort_;
	ev_io * readWatcher_;
	ev_io * writeWatcher_;
	QByteArray readBuf_;
	QByteArray writeBuf_;

	friend class TCPServer;
};

}}	// namespace

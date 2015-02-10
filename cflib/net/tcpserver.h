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

namespace cflib { namespace crypt { class TLSCredentials; }}
namespace cflib { namespace crypt { class TLSStream;      }}

namespace cflib { namespace net {

class TCPConnInitializer;

class TCPServer
{
	Q_DISABLE_COPY(TCPServer)
public:
	TCPServer();
	TCPServer(crypt::TLSCredentials & credentials);
	virtual ~TCPServer();

	static int openListenSocket(quint16 port, const QByteArray & ip = "127.0.0.1");
	bool start(int listenSocket);
	bool start(quint16 port, const QByteArray & ip = "127.0.0.1");
	bool stop();
	bool isRunning() const;

	const TCPConnInitializer * openConnection(const QByteArray & destIP, quint16 destPort);
	const TCPConnInitializer * openConnection(const QByteArray & destIP, quint16 destPort,
		crypt::TLSCredentials & credentials);

	static TCPServer * instance() { return instance_; }

protected:
	virtual void newConnection(const TCPConnInitializer * init) = 0;

private:
	class Impl;
	Impl * impl_;
	static TCPServer * instance_;
	friend class TCPConn;
	friend class TCPConnInitializer;
};

class TCPConn
{
	Q_DISABLE_COPY(TCPConn)
public:
	TCPConn(const TCPConnInitializer * init);
	virtual ~TCPConn();

	QByteArray read();
	void write(const QByteArray & data);

	void closeNicely();
	void abortConnection();

	void startWatcher();

	bool isClosed() const { return socket_ == -1; }

	QByteArray peerIP() const { return peerIP_; }
	quint16 peerPort() const { return peerPort_; }

	const TCPConnInitializer * detachFromSocket();

	void setNoDelay(bool noDelay);

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
	bool closeAfterWriting_;
	crypt::TLSStream * tlsStream_;

	friend class TCPServer;
};

}}	// namespace

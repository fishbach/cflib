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

#include <cflib/net/tcpconn.h>
#include <cflib/net/tcpmanager.h>
#include <cflib/util/threadverify.h>

namespace cflib { namespace crypt { class TLSCredentials; }}

namespace cflib { namespace net {

class TLSThread;

class TCPManager::Impl : public util::ThreadVerify
{
public:
	Impl(TCPManager & parent, uint tlsThreadCount);
	~Impl();

	bool isRunning() const { return listenSock_ != -1; }
	bool start(int listenSocket, crypt::TLSCredentials * credentials);
	bool stop();

	const TCPConnInitializer * openConnection(const QByteArray & destIP, quint16 destPort,
		const QByteArray & sourceIP, quint16 sourcePort,
		crypt::TLSCredentials * credentials);

	void startReadWatcher(TCPConn * conn);
	void writeToSocket(TCPConn * conn, const QByteArray & data, bool notifyFinished);
	void closeConn(TCPConn * conn, TCPConn::CloseType type);
	void destroy(TCPConn * conn);
	void deleteConn(TCPConn * conn);

	void tlsWrite(TCPConn * conn, const QByteArray & data, bool notifyFinished) const;
	void tlsCloseConn(TCPConn * conn, TCPConn::CloseType type) const;

	static void setNoDelay(int socket, bool noDelay);
	static int openListenSocket(const QByteArray & ip, quint16 port);

	static void readable(ev_loop * loop, ev_io * w, int revents);
	static void writeable(ev_loop * loop, ev_io * w, int revents);

protected:
	virtual void deleteThreadData();

private:
	static void listenSocketReadable(ev_loop * loop, ev_io * w, int revents);

private:
	TCPManager & parent_;
	int listenSock_;
	bool isIPv6Sock_;
	ev_io * readWatcher_;
	crypt::TLSCredentials * credentials_;
	const uint tlsThreadCount_;
	QVector<TLSThread *> tlsThreads_;
	uint tlsConnId_;
};

}}	// namespace

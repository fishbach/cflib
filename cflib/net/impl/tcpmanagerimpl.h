/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/crypt/tlscredentials.h>
#include <cflib/net/tcpconn.h>
#include <cflib/util/threadverify.h>

struct ev_io;

namespace cflib { namespace net {

class TCPManager;

namespace impl {

class TLSThread;

class TCPManagerImpl : public util::ThreadVerify
{
public:
	TCPManagerImpl(TCPManager & parent, uint tlsThreadCount);
	TCPManagerImpl(TCPManager & parent, uint tlsThreadCount, util::ThreadVerify * other);
	~TCPManagerImpl();

	bool isRunning() const { return listenSock_ != -1; }
	bool start(int listenSocket, crypt::TLSCredentials * credentials);
	void stop();

	TCPConnData * openConnection(const QByteArray & destAddress, quint16 destPort,
		const QByteArray & sourceIP, quint16 sourcePort,
		crypt::TLSCredentials * credentials, bool preferIPv6);

	void startReadWatcher(TCPConnData * conn);
	void writeToSocket(TCPConnData * conn, const QByteArray & data, bool notifyFinished);
	void closeConn(TCPConnData * conn, TCPConn::CloseType type, bool notifyClose);
	void deleteOnFinish(TCPConnData * conn);

	void tlsStartReadWatcher(TCPConnData * conn);
	void tlsWrite(TCPConnData * conn, const QByteArray & data, bool notifyFinished) const;
	void tlsCloseConn(TCPConnData * conn, TCPConn::CloseType type, bool notifyClose) const;
	void tlsDeleteOnFinish(TCPConnData * conn) const;

	static void setNoDelay(int socket, bool noDelay);
	static int openListenSocket(const QByteArray & ip, quint16 port);

	static void readable(ev_loop * loop, ev_io * w, int revents);
	static void writeable(ev_loop * loop, ev_io * w, int revents);

	TCPManager & parent;

	crypt::TLSCredentials clientCredentials;

protected:
	virtual void deleteThreadData();

private:
	static void listenSocketReadable(ev_loop * loop, ev_io * w, int revents);
	void callClosed(TCPConnData * conn);
	TCPConnData * addConnection(int sock, const QByteArray & destIP, quint16 destPort,
		crypt::TLSCredentials * credentials, const QByteArray & destAddress);

private:
	int listenSock_;
	bool isIPv6Sock_;
	ev_io * readWatcher_;
	crypt::TLSCredentials * credentials_;
	QVector<TLSThread *> tlsThreads_;
	QAtomicInteger<uint> tlsConnId_;
	QSet<TCPConnData *> connections_;
};

}}}	// namespace

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

#include "tcpmanagerimpl.h"

#include <cflib/crypt/tlsclient.h>
#include <cflib/crypt/tlsserver.h>
#include <cflib/crypt/tlssessions.h>
#include <cflib/net/impl/tcpconndata.h>
#include <cflib/net/impl/tlsthread.h>
#include <cflib/net/tcpmanager.h>
#include <cflib/util/libev.h>
#include <cflib/util/log.h>

#include <errno.h>
#include <fcntl.h>
#include <functional>
#include <string.h>
#include <sys/types.h>

#ifndef Q_OS_WIN
	#include <arpa/inet.h>
	#include <netinet/in.h>
	#include <netinet/tcp.h>
	#include <sys/socket.h>
	#include <unistd.h>
#else
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#define ssize_t int
	#define socklen_t int
	#define SHUT_RD   SD_RECEIVE
	#define SHUT_WR   SD_SEND
	#define SHUT_RDWR SD_BOTH
	#define close closesocket
#endif

USE_LOG(LogCat::Network)

using namespace cflib::crypt;
using namespace cflib::util;

namespace cflib { namespace net { namespace impl {

namespace {

Q_GLOBAL_STATIC(TLSSessions, tlsSessions)

inline bool setNonBlocking(int fd)
{
#ifdef Q_OS_WIN
	unsigned long nonblocking = 1;
	ioctlsocket(fd, FIONBIO, (unsigned long *)&nonblocking);
#else
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
		logWarn("cannot set fd %1 non blocking (%2 - %3)", fd, errno, strerror(errno));
		return false;
	}
#endif
	return true;
}

inline bool callWithSockaddr(const QByteArray & ip, quint16 port, std::function<bool (const struct sockaddr *, socklen_t)> func)
{
	if (ip.indexOf('.') == -1) {
		struct sockaddr_in6 addr;
		memset(&addr, 0, sizeof(addr));
		addr.sin6_family = AF_INET6;
		if (inet_pton(AF_INET6, ip.constData(), &addr.sin6_addr) == 0) return false;
		addr.sin6_port = htons(port);
		return func((struct sockaddr *)&addr, sizeof(addr));
	} else {
		struct sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		if (inet_pton(AF_INET, ip.constData(), &addr.sin_addr) == 0) return false;
		addr.sin_port = htons(port);
		return func((struct sockaddr *)&addr, sizeof(addr));
	}
}

}

TCPManagerImpl::TCPManagerImpl(TCPManager & parent, uint tlsThreadCount) :
	ThreadVerify("TCPManager", ThreadVerify::Net),
	parent(parent),
	listenSock_(-1),
	isIPv6Sock_(false),
	readWatcher_(new ev_io),
	credentials_(0),
	tlsConnId_(0)
{
	setThreadPrio(QThread::HighestPriority);
	for (uint i = 1 ; i <= tlsThreadCount ; ++i) tlsThreads_.append(new TLSThread(*this, i, tlsThreadCount));
}

TCPManagerImpl::~TCPManagerImpl()
{
	logFunctionTrace
	stopVerifyThread();
	foreach (TLSThread * th, tlsThreads_) delete th;
	delete readWatcher_;
}

void TCPManagerImpl::deleteThreadData()
{
	if (isRunning()) stop();
	foreach (TCPConnData * c, connections_) {
		if (c->tlsStream) tlsCloseConn(c, TCPConn::HardClosed, false);
		else              closeConn(c, TCPConn::HardClosed, false);
	}
}

bool TCPManagerImpl::start(int listenSocket, crypt::TLSCredentials * credentials)
{
	SyncedThreadCall<bool> stc(this);
	if (!stc.verify(&TCPManagerImpl::start, listenSocket, credentials)) return stc.retval();

	if (listenSocket < 0) return false;

	if (isRunning()) {
		logWarn("server already running");
		return false;
	}

	if (credentials && tlsThreads_.isEmpty()) {
		logWarn("no TLS threads");
		return false;
	}

	struct sockaddr address;
	socklen_t len = sizeof(address);
	if (getsockname(listenSocket, &address, &len) < 0) {
		logWarn("invalid socket %1 (%2 - %3)", listenSocket, errno, strerror(errno));
		return false;
	}

	listenSock_ = listenSocket;
	credentials_ = credentials;
	isIPv6Sock_ = address.sa_family == AF_INET6;

	// watching for incoming activity
	ev_io_init(readWatcher_, &TCPManagerImpl::listenSocketReadable, listenSock_, EV_READ);
	readWatcher_->data = this;
	ev_io_start(libEVLoop(), readWatcher_);

	logInfo("server started");
	return true;
}

bool TCPManagerImpl::stop()
{
	SyncedThreadCall<bool> stc(this);
	if (!stc.verify(&TCPManagerImpl::stop)) return stc.retval();

	if (!isRunning()) {
		logWarn("server not running");
		return false;
	}

	ev_io_stop(libEVLoop(), readWatcher_);
	close(listenSock_);
	listenSock_ = -1;
	credentials_ = 0;

	logInfo("server stopped");
	return true;
}

TCPConnData * TCPManagerImpl::openConnection(
	const QByteArray & destIP, quint16 destPort,
	const QByteArray & sourceIP, quint16 sourcePort,
	TLSCredentials * credentials)
{
	// no thread verify needed here

	if (credentials && tlsThreads_.isEmpty()) {
		logWarn("no TLS threads");
		return 0;
	}

	// create non blocking socket
	int sock = socket(destIP.indexOf('.') == -1 ? AF_INET6 : AF_INET, SOCK_STREAM, 0);
	if (sock < 0) return 0;
	if (!setNonBlocking(sock)) {
		close(sock);
		return 0;
	}

	// bind source address and port
	if (!sourceIP.isEmpty()) {
		{ int on = 1; setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&on, sizeof(on)); }
		if (!callWithSockaddr(sourceIP, sourcePort, [sock](const struct sockaddr * addr, socklen_t addrlen) {
			return bind(sock, addr, addrlen) >= 0; }))
		{
			logInfo("cannot bind source %1:%2 (%3 - %4)", sourceIP, sourcePort, errno, strerror(errno));
			close(sock);
			return 0;
		}
	}

	// bind destination address and port
	if (!callWithSockaddr(destIP, destPort, [sock](const struct sockaddr * addr, socklen_t addrlen) {
		return connect(sock, addr, addrlen) >= 0 || errno == EINPROGRESS; }))
	{
		logInfo("cannot connect source %1:%2 (%3 - %4)", destIP, destPort, errno, strerror(errno));
		close(sock);
		return 0;
	}

	logDebug("opened connection %1 to %2:%3", sock, destIP, destPort);

	TCPConnData * conn = credentials ?
		new TCPConnData(*this, sock, destIP, destPort,
			new TLSClient(*tlsSessions(), *credentials), ++tlsConnId_ % tlsThreads_.size()) :
		new TCPConnData(*this, sock, destIP, destPort, 0, 0);
	connections_ << conn;
	return conn;
}

void TCPManagerImpl::startReadWatcher(TCPConnData * conn)
{
	if (!verifyThreadCall(&TCPManagerImpl::startReadWatcher, conn)) return;

	if (conn->closeType & TCPConn::ReadClosed) {
		callClosed(conn);
		return;
	}
	ev_io_start(libEVLoop(), conn->readWatcher);
}

void TCPManagerImpl::writeToSocket(TCPConnData * conn, const QByteArray & data, bool notifyFinished)
{
	if (!verifyThreadCall(&TCPManagerImpl::writeToSocket, conn, data, notifyFinished)) return;

	conn->writeBuf += data;

	if (conn->writeBuf.isEmpty()) {
		if (notifyFinished) execCall(new Functor0<TCPConn>(conn->conn, &TCPConn::writeFinished));
		return;
	}

	if (conn->closeType & TCPConn::WriteClosed) {
		conn->writeBuf.clear();
		if (notifyFinished) callClosed(conn);
		return;
	}

	if (notifyFinished) conn->notifyWrite = true;
	if (!ev_is_active(conn->writeWatcher)) writeable(libEVLoop(), conn->writeWatcher, 0);
}

void TCPManagerImpl::closeConn(TCPConnData * conn, TCPConn::CloseType type, bool notifyClose)
{
	if (!verifyThreadCall(&TCPManagerImpl::closeConn, conn, type, notifyClose)) return;

	// update state
	volatile TCPConn::CloseType & ct = conn->closeType;
	const TCPConn::CloseType oldCt = ct;
	ct = (TCPConn::CloseType)(ct | type);

	// nothing changed
	if (oldCt == ct) return;

	// calc changes
	const TCPConn::CloseType changed = (TCPConn::CloseType)(oldCt ^ ct);
	bool readClosed  = changed & TCPConn::ReadClosed;
	bool writeClosed = changed & TCPConn::WriteClosed;

	if (ct != TCPConn::HardClosed && writeClosed && !conn->writeBuf.isEmpty()) {
		ct = (TCPConn::CloseType)(ct & ~TCPConn::WriteClosed);
		writeClosed = false;
		conn->notifySomeBytesWritten = false;
		conn->closeAfterWriting = true;
		if (oldCt == ct) return;
	}

	logDebug("socket %1 closed (%2 => %3)", conn->socket, (int)oldCt, (int)ct);

	// stop watcher
	if (readClosed && ev_is_active(conn->readWatcher)) {
		ev_io_stop(libEVLoop(), conn->readWatcher);
		notifyClose = true;
	}
	if (writeClosed) {
		if (ev_is_active(conn->writeWatcher)) ev_io_stop(libEVLoop(), conn->writeWatcher);
		if (conn->notifyWrite) {
			conn->notifyWrite = false;
			notifyClose = true;
		}
	}
	if (notifyClose) callClosed(conn);

	// hard close and nothing to do
	if (!readClosed && !writeClosed) return;

	// close socket
	if (readClosed)  conn->readBuf.clear();
	if (ct == TCPConn::HardClosed) {
		// send RST instead of FIN
		struct linger lin;
		lin.l_onoff  = 1;
		lin.l_linger = 0;
		setsockopt(conn->socket, SOL_SOCKET, SO_LINGER, (const void *)&lin, sizeof(lin));
		close(conn->socket);
		logDebug("fd %1 closed", conn->socket);
	} else if (ct & TCPConn::ReadClosed) {
		if (ct & TCPConn::WriteClosed) {
			// no need to call shutdown(conn->socket, SHUT_RDWR) - close() will do.
			close(conn->socket);
			logDebug("fd %1 closed", conn->socket);
		} else {
			shutdown(conn->socket, SHUT_RD);
		}
	} else {
		shutdown(conn->socket, SHUT_WR);
	}
}

void TCPManagerImpl::deleteOnFinish(TCPConnData * conn)
{
	if (!verifyThreadCall(&TCPManagerImpl::deleteOnFinish, conn)) return;

	if (ev_is_active(conn->writeWatcher)) {
		closeConn(conn, TCPConn::ReadClosed, false);
		conn->notifySomeBytesWritten = false;
		conn->closeAfterWriting = true;
		conn->deleteAfterWriting = true;
	} else {
		closeConn(conn, TCPConn::ReadWriteClosed, false);
		connections_.remove(conn);
		delete conn;
	}
}

void TCPManagerImpl::tlsWrite(TCPConnData * conn, const QByteArray & data, bool notifyFinished) const
{
	tlsThreads_[conn->tlsThreadId]->write(conn, data, notifyFinished);
}

void TCPManagerImpl::tlsCloseConn(TCPConnData * conn, TCPConn::CloseType type, bool notifyClose) const
{
	tlsThreads_[conn->tlsThreadId]->closeConn(conn, type, notifyClose);
}

void TCPManagerImpl::tlsDeleteOnFinish(TCPConnData * conn) const
{
	tlsThreads_[conn->tlsThreadId]->deleteOnFinish(conn);
}

void TCPManagerImpl::setNoDelay(int socket, bool noDelay)
{
	int on = noDelay ? 1 : 0;
	setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof(on));
}

int TCPManagerImpl::openListenSocket(const QByteArray & ip, quint16 port)
{
	// create non blocking socket
	int rv = socket(ip.indexOf('.') == -1 ? AF_INET6 : AF_INET, SOCK_STREAM, 0);
	if (rv < 0) return -1;
	if (!setNonBlocking(rv)) {
		close(rv);
		return -1;
	}

	// bind to address and port
	{ int on = 1; setsockopt(rv, SOL_SOCKET, SO_REUSEADDR, (const void *)&on, sizeof(on)); }
	if (!callWithSockaddr(ip, port, [rv](const struct sockaddr * addr, socklen_t addrlen) {
		return bind(rv, addr, addrlen) >= 0; }))
	{
		logInfo("cannot bind to %1:%2 (%3 - %4)", ip, port, errno, strerror(errno));
		close(rv);
		return -1;
	}

	// start listening
	if (listen(rv, 1024) < 0) {
		close(rv);
		return -1;
	}

	return rv;
}

void TCPManagerImpl::readable(ev_loop * loop, ev_io * w, int)
{
	TCPConnData * conn = (TCPConnData *)w->data;
	TCPManagerImpl & impl = conn->impl;

	char * data = (char *)conn->readBuf.constData();
	const int fd = conn->socket;

	const ssize_t count = ::recv(fd, data, conn->readBuf.size(), 0);
	if (count > 0) {
		logTrace("read %1 raw bytes from %2", (int)count, fd);
		ev_io_stop(loop, w);
		conn->readData.append(data, (int)count);
		if (!conn->tlsStream) conn->conn->newBytesAvailable();
		else                  impl.tlsThreads_[conn->tlsThreadId]->read(conn);
		return;
	}

	if (count == 0) {
		logDebug("read channel closed on fd %1", fd);
		impl.closeConn(conn, TCPConn::ReadClosed, false);
		return;
	}

	if (errno == EAGAIN || errno == EWOULDBLOCK) {
		logDebug("nothing to read on fd %1 (%2 - %3)", fd, errno, strerror(errno));
		return;
	}

	logInfo("read on fd %1 failed (%2 - %3)", fd, errno, strerror(errno));
	impl.closeConn(conn, TCPConn::HardClosed, false);
}

void TCPManagerImpl::writeable(ev_loop * loop, ev_io * w, int)
{
	TCPConnData * conn = (TCPConnData *)w->data;
	TCPManagerImpl & impl = conn->impl;

	QByteArray & buf = conn->writeBuf;
	const int fd = conn->socket;

	const ssize_t count = ::send(fd, buf.constData(), buf.size(), 0);
	logTrace("wrote %1 / %2 bytes on %3", (qint64)count, buf.size(), fd);
	if (count < buf.size()) {
		if (count < 0 && errno != EAGAIN && errno != EWOULDBLOCK && errno != ENOTCONN) {
			logDebug("write on fd %1 failed (%2 - %3)", fd, errno, strerror(errno));
			buf.clear();
			impl.closeConn(conn, errno == EPIPE ? TCPConn::WriteClosed : TCPConn::HardClosed, false);
			if (conn->deleteAfterWriting) {
				impl.connections_.remove(conn);
				delete conn;
			}
			return;
		}
		if (count > 0) {
			buf.remove(0, count);
			if (conn->notifySomeBytesWritten) {
				impl.execCall(new Functor1<TCPConn, quint64>(conn->conn, &TCPConn::someBytesWritten, (quint64)count));
			}
		}
		if (!ev_is_active(w)) ev_io_start(loop, w);
	} else {
		buf.clear();
		if (conn->closeAfterWriting) {
			impl.closeConn(conn, TCPConn::WriteClosed, false);
			if (conn->deleteAfterWriting) {
				impl.connections_.remove(conn);
				delete conn;
			}
		} else {
			if (ev_is_active(w)) ev_io_stop(loop, w);
			if (conn->notifySomeBytesWritten) {
				impl.execCall(new Functor1<TCPConn, quint64>(conn->conn, &TCPConn::someBytesWritten, (quint64)count));
			}
			if (conn->notifyWrite) {
				conn->notifyWrite = false;
				impl.execCall(new Functor0<TCPConn>(conn->conn, &TCPConn::writeFinished));
			}
		}
	}
}

void TCPManagerImpl::listenSocketReadable(ev_loop *, ev_io * w, int)
{
	logFunctionTrace

	TCPManagerImpl * impl = (TCPManagerImpl *)w->data;

	forever {
		// get socket and source address
		int newSock;
		char ip[40];
		quint16 port;
		if (impl->isIPv6Sock_) {
			struct sockaddr_in6 cliAddr;
			socklen_t len = sizeof(cliAddr);
			newSock = accept(impl->listenSock_, (struct sockaddr *)&cliAddr, &len);
			if (newSock < 0) break;
			inet_ntop(AF_INET6, &cliAddr.sin6_addr, ip, sizeof(ip));
			port = ntohs(cliAddr.sin6_port);
		} else {
			struct sockaddr_in cliAddr;
			socklen_t len = sizeof(cliAddr);
			newSock = accept(impl->listenSock_, (struct sockaddr *)&cliAddr, &len);
			if (newSock < 0) break;
			inet_ntop(AF_INET, &cliAddr.sin_addr, ip, sizeof(ip));
			port = ntohs(cliAddr.sin_port);
		}
		setNonBlocking(newSock);

		logDebug("new connection (%1) from %2:%3", newSock, ip, port);

		TCPConnData * conn = impl->credentials_ ?
			new TCPConnData(*impl, newSock, ip, port,
				new TLSServer(*tlsSessions(), *(impl->credentials_)),
				++impl->tlsConnId_ % impl->tlsThreads_.size()) :
			new TCPConnData(*impl, newSock, ip, port, 0, 0);
		impl->connections_ << conn;
		impl->parent.newConnection(conn);
	}
}

void TCPManagerImpl::callClosed(TCPConnData * conn)
{
	if (conn->lastInformedCloseType == conn->closeType) return;
	conn->lastInformedCloseType = conn->closeType;
	if (conn->tlsStream) tlsThreads_[conn->tlsThreadId]->callClosed(conn);
	else                 execCall(new Functor0<TCPConnData>(conn, &TCPConnData::callClosed));
}

}}}	// namespace

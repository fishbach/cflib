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
#include <cflib/net/impl/tcpconninitializer.h>
#include <cflib/net/impl/tlsthread.h>
#include <cflib/util/libev.h>
#include <cflib/util/log.h>

#include <errno.h>
#include <fcntl.h>
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

namespace cflib { namespace net {

namespace {

inline bool setNonBlocking(int fd)
{
#ifdef Q_OS_WIN
	unsigned long nonblocking = 1;
	ioctlsocket(fd, FIONBIO, (unsigned long *)&nonblocking);
#else
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
		logWarn("cannot set fd %1 non blocking (errno: %2)", fd, errno);
		return false;
	}
#endif
	return true;
}

}

TCPManager::Impl::Impl(TCPManager & parent, TLSCredentials * credentials, uint tlsThreadCount) :
	ThreadVerify("TCPServer", ThreadVerify::Net),
	parent_(parent),
	listenSock_(-1),
	readWatcher_(new ev_io),
	sessions_(credentials ? new TLSSessions : 0),
	credentials_(credentials),
	tlsConnId_(0)
{
	setThreadPrio(QThread::HighestPriority);
	if (credentials) {
		if (tlsThreadCount == 0) logCritical("thread count cannot be 0 for worker thread TLSThread");
		for (uint i = 1 ; i <= tlsThreadCount ; ++i) tlsThreads_.append(new TLSThread(*this, i, tlsThreadCount));
	}
}

TCPManager::Impl::~Impl()
{
	foreach (TLSThread * th, tlsThreads_) delete th;
	stopVerifyThread();
	delete sessions_;
	delete readWatcher_;
}

void TCPManager::Impl::deleteThreadData()
{
	if (isRunning()) stop();
}

bool TCPManager::Impl::start(int listenSocket)
{
	SyncedThreadCall<bool> stc(this);
	if (!stc.verify(&Impl::start, listenSocket)) return stc.retval();

	if (listenSocket < 0) return false;

	if (isRunning()) {
		logWarn("server already running");
		return false;
	}
	listenSock_ = listenSocket;

	// watching for incoming activity
	ev_io_init(readWatcher_, &Impl::listenSocketReadable, listenSock_, EV_READ);
	readWatcher_->data = this;
	ev_io_start(libEVLoop(), readWatcher_);

	logInfo("server started");
	return true;
}

bool TCPManager::Impl::stop()
{
	SyncedThreadCall<bool> stc(this);
	if (!stc.verify(&Impl::stop)) return stc.retval();

	if (!isRunning()) {
		logWarn("server not running");
		return false;
	}

	ev_io_stop(libEVLoop(), readWatcher_);
	close(listenSock_);
	listenSock_ = -1;

	logInfo("server stopped");
	return true;
}

const TCPConnInitializer * TCPManager::Impl::openConnection(
	const QByteArray & destIP, quint16 destPort,
	const QByteArray & sourceIP, quint16 sourcePort,
	TLSCredentials * credentials)
{
	// IPv6
	const bool isIPv6 = destIP.indexOf('.') == -1;

	// create socket
	int sock = socket(isIPv6 ? AF_INET6 : AF_INET, SOCK_STREAM, 0);
	if (sock < 0) return 0;

	if (!setNonBlocking(sock)) {
		close(sock);
		return 0;
	}

	// bind source address and port
	if (!sourceIP.isEmpty()) {
		{ int on = 1; setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(on)); }
		if (isIPv6) {
			struct sockaddr_in6 sourceAddr;
			sourceAddr.sin6_family = AF_INET6;
			if (inet_pton(AF_INET6, sourceIP.constData(), &sourceAddr.sin6_addr) == 0) {
				close(sock);
				return 0;
			}
			sourceAddr.sin6_port = htons(sourcePort);
			if (bind(sock, (struct sockaddr *)&sourceAddr, sizeof(sourceAddr)) < 0) {
				close(sock);
				return 0;
			}
		} else {
			struct sockaddr_in sourceAddr;
			sourceAddr.sin_family = AF_INET;
			if (inet_pton(AF_INET, sourceIP.constData(), &sourceAddr.sin_addr) == 0) {
				close(sock);
				return 0;
			}
			sourceAddr.sin_port = htons(sourcePort);
			if (bind(sock, (struct sockaddr *)&sourceAddr, sizeof(sourceAddr)) < 0) {
				close(sock);
				return 0;
			}
		}
	}

	// bind destination address and port
	if (isIPv6) {
		struct sockaddr_in6 destAddr;
		memset(&destAddr, 0, sizeof(destAddr));
		destAddr.sin6_family = AF_INET6;
		if (inet_pton(AF_INET6, destIP.constData(), &destAddr.sin6_addr) == 0) {
			close(sock);
			return 0;
		}
		destAddr.sin6_port = htons(destPort);
		if (connect(sock, (struct sockaddr *)&destAddr, sizeof(destAddr)) < 0 && errno != EINPROGRESS) {
			logDebug("connect failed with errno: %1", errno);
			close(sock);
			return 0;
		}
	} else {
		struct sockaddr_in destAddr;
		memset(&destAddr, 0, sizeof(destAddr));
		destAddr.sin_family = AF_INET;
		if (inet_pton(AF_INET, destIP.constData(), &destAddr.sin_addr) == 0) {
			close(sock);
			return 0;
		}
		destAddr.sin_port = htons(destPort);
		if (connect(sock, (struct sockaddr *)&destAddr, sizeof(destAddr)) < 0 && errno != EINPROGRESS) {
			logDebug("connect failed with errno: %1", errno);
			close(sock);
			return 0;
		}
	}

	const TCPConnInitializer * ci = credentials ?
			new TCPConnInitializer(parent_, sock, destIP, destPort,
				new TLSClient(*sessions_, *credentials), ++tlsConnId_ % tlsThreads_.size()) :
			new TCPConnInitializer(parent_, sock, destIP, destPort, 0, 0);

	logDebug("opened connection %1 to %2:%3", sock, ci->peerIP, ci->peerPort);
	return ci;
}

void TCPManager::Impl::startReadWatcher(TCPConn * conn)
{
	if (!verifyThreadCall(&Impl::startReadWatcher, conn)) return;

	const TCPConn::CloseType ct = conn->closeType_;
	if (ct & TCPConn::ReadClosed) {
		execCall(new Functor1<TCPConn, TCPConn::CloseType>(conn, &TCPConn::closed, ct));
		return;
	}
	ev_io_start(libEVLoop(), conn->readWatcher_);
}

void TCPManager::Impl::writeToSocket(TCPConn * conn, const QByteArray & data, bool notifyFinished)
{
	if (!verifyThreadCall(&Impl::writeToSocket, conn, data, notifyFinished)) return;

	conn->writeBuf_ += data;

	if (conn->writeBuf_.isEmpty()) {
		if (notifyFinished) execCall(new Functor0<TCPConn>(conn, &TCPConn::writeFinished));
		return;
	}

	if (conn->closeType_ & TCPConn::WriteClosed) {
		conn->writeBuf_.clear();
		execCall(new Functor1<TCPConn, TCPConn::CloseType>(conn, &TCPConn::closed, conn->closeType_));
		return;
	}

	if (notifyFinished) conn->notifyWrite_ = true;
	if (!ev_is_active(conn->writeWatcher_)) writeable(libEVLoop(), conn->writeWatcher_, 0);
}

void TCPManager::Impl::closeConn(TCPConn * conn, TCPConn::CloseType type)
{
	if (!verifyThreadCall(&Impl::closeConn, conn, type)) return;

	// update state
	TCPConn::CloseType & ct = conn->closeType_;
	const TCPConn::CloseType oldCt = ct;
	ct = (TCPConn::CloseType)(ct | type);
	if (oldCt == ct) return;

	// calc changes
	const TCPConn::CloseType changed = (TCPConn::CloseType)(oldCt ^ ct);
	bool readClosed  = changed & TCPConn::ReadClosed;
	bool writeClosed = changed & TCPConn::WriteClosed;

	if (ct != TCPConn::HardClosed && writeClosed && !conn->writeBuf_.isEmpty()) {
		ct = (TCPConn::CloseType)(ct & ~TCPConn::WriteClosed);
		writeClosed = false;
		conn->closeAfterWriting_ = true;
		if (oldCt == ct) return;
	}

	logDebug("socket %1 closed (%2 => %3)", conn->socket_, (int)oldCt, (int)ct);

	// stop watcher
	bool wasWatching = false;
	if (readClosed && ev_is_active(conn->readWatcher_)) {
		ev_io_stop(libEVLoop(), conn->readWatcher_);
		wasWatching = true;
	}
	if (writeClosed && ev_is_active(conn->writeWatcher_)) {
		ev_io_stop(libEVLoop(), conn->writeWatcher_);
		wasWatching = true;
	}
	if (wasWatching) execCall(new Functor1<TCPConn, TCPConn::CloseType>(conn, &TCPConn::closed, ct));

	// close socket
	if (ct == TCPConn::HardClosed) {
		close(conn->socket_);
		conn->socket_ = -1;
		conn->readBuf_.clear();
	} else {
		if (readClosed) {
			conn->readBuf_.clear();
			shutdown(conn->socket_, writeClosed ? SHUT_RDWR : SHUT_RD);
		} else {
			shutdown(conn->socket_, SHUT_WR);
		}
	}
}

void TCPManager::Impl::destroy(TCPConn * conn)
{
	if (!verifyThreadCall(&Impl::destroy, conn)) return;

	if (conn->closeType_ != TCPConn::HardClosed) {
		conn->closeType_ = TCPConn::HardClosed;
		ev_io_stop(libEVLoop(), conn->readWatcher_);
		ev_io_stop(libEVLoop(), conn->writeWatcher_);
		close(conn->socket_);
	}

	if (conn->tlsStream_) tlsThreads_[conn->tlsThreadId_]->destroy(conn);
	else                  delete conn;
}

void TCPManager::Impl::deleteConn(TCPConn * conn)
{
	if (!verifyThreadCall(&Impl::deleteConn, conn)) return;
	delete conn;
}

void TCPManager::Impl::tlsWrite(TCPConn * conn, const QByteArray & data, bool notifyFinished) const
{
	tlsThreads_[conn->tlsThreadId_]->write(conn, data, notifyFinished);
}

void TCPManager::Impl::tlsCloseConn(TCPConn * conn, TCPConn::CloseType type) const
{
	tlsThreads_[conn->tlsThreadId_]->closeConn(conn, type);
}

void TCPManager::Impl::setNoDelay(int socket, bool noDelay)
{
	int on = noDelay ? 1 : 0;
	setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof(on));
}

int TCPManager::Impl::openListenSocket(const QByteArray & ip, quint16 port)
{
	// create socket
	int rv = socket(AF_INET, SOCK_STREAM, 0);
	if (rv < 0) return -1;

	if (!setNonBlocking(rv)) {
		close(rv);
		return -1;
	}
	{ int on = 1; setsockopt(rv, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(on)); }

	// bind to address and port
	struct sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	if (inet_pton(AF_INET, ip.constData(), &servAddr.sin_addr) == 0) {
		close(rv);
		return -1;
	}
	servAddr.sin_port = htons(port);
	if (bind(rv, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
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

void TCPManager::Impl::readable(ev_loop * loop, ev_io * w, int)
{
	TCPConn * conn = (TCPConn *)w->data;
	Impl & impl = *conn->mgr_.impl_;

	char * data = (char *)conn->readBuf_.constData();
	const ssize_t count = ::recv(conn->socket_, data, conn->readBuf_.size(), 0);
	if (count > 0) {
		logTrace("read %1 raw bytes", (int)count);
		ev_io_stop(loop, w);
		conn->readData_.append(data, (int)count);
		if (!conn->tlsStream_) conn->newBytesAvailable();
		else                   impl.tlsThreads_[conn->tlsThreadId_]->read(conn);
		return;
	}

	if (count == 0) {
		logDebug("read channel closed on fd %1", conn->socket_);
		impl.closeConn(conn, TCPConn::ReadClosed);
		return;
	}

	if (errno == EAGAIN || errno == EWOULDBLOCK) {
		logDebug("nothing to read on fd %1 (errno: %2)", conn->socket_, errno);
		return;
	}

	logInfo("read on fd %1 failed (errno: %2)", conn->socket_, errno);
	impl.closeConn(conn, TCPConn::HardClosed);
}

void TCPManager::Impl::writeable(ev_loop * loop, ev_io * w, int)
{
	TCPConn * conn = (TCPConn *)w->data;
	Impl & impl = *conn->mgr_.impl_;

	QByteArray & buf = conn->writeBuf_;
	const int fd = conn->socket_;

	ssize_t count = ::send(fd, buf.constData(), buf.size(), 0);
	logTrace("wrote %1 / %2 bytes", (qint64)count, buf.size());
	if (count < buf.size()) {
		if (count < 0 && errno != EAGAIN && errno != EWOULDBLOCK && errno != ENOTCONN) {
			logDebug("write on fd %1 failed (errno: %2)", fd, errno);
			buf.clear();
			if (ev_is_active(w)) ev_io_stop(loop, w);
			impl.closeConn(conn, errno == EPIPE ? TCPConn::WriteClosed : TCPConn::HardClosed);
			impl.execCall(new Functor1<TCPConn, TCPConn::CloseType>(conn, &TCPConn::closed, conn->closeType_));
			return;
		}
		if (count > 0) buf.remove(0, count);
		if (!ev_is_active(w)) ev_io_start(loop, w);
	} else {
		buf.clear();
		if (ev_is_active(w)) ev_io_stop(loop, w);
		if (conn->closeAfterWriting_ && !(conn->closeType_ & TCPConn::WriteClosed))
			impl.closeConn(conn, TCPConn::WriteClosed);
		if (conn->notifyWrite_) {
			conn->notifyWrite_ = false;
			impl.execCall(new Functor0<TCPConn>(conn, &TCPConn::writeFinished));
		}
	}
}

void TCPManager::Impl::listenSocketReadable(ev_loop *, ev_io * w, int)
{
	logFunctionTrace

	Impl * impl = (Impl *)w->data;
	forever {
		struct sockaddr_in cliAddr;
		socklen_t len = sizeof(cliAddr);
		int newSock = accept(impl->listenSock_, (struct sockaddr *)&cliAddr, &len);
		if (newSock < 0) break;

		setNonBlocking(newSock);

		char ipAddr[16];
		const TCPConnInitializer * ci = impl->credentials_ ?
			new TCPConnInitializer(impl->parent_,
				newSock, inet_ntop(AF_INET, &cliAddr.sin_addr, ipAddr, sizeof(ipAddr)), ntohs(cliAddr.sin_port),
				new TLSServer(*(impl->sessions_), *(impl->credentials_)),
				++impl->tlsConnId_ % impl->tlsThreads_.size()) :
			new TCPConnInitializer(impl->parent_,
				newSock, inet_ntop(AF_INET, &cliAddr.sin_addr, ipAddr, sizeof(ipAddr)), ntohs(cliAddr.sin_port),
				0, 0);
		logDebug("new connection (%1) from %2:%3", newSock, ci->peerIP, ci->peerPort);
		impl->parent_.newConnection(ci);
	}
}

}}	// namespace

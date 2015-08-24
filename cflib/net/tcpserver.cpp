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

#include "tcpserver.h"

#include <cflib/crypt/tlsclient.h>
#include <cflib/crypt/tlsserver.h>
#include <cflib/crypt/tlssessions.h>
#include <cflib/util/libev.h>
#include <cflib/util/log.h>
#include <cflib/util/threadverify.h>

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

using namespace cflib::crypt;
using namespace cflib::util;

USE_LOG(LogCat::Network)

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

class TCPConnInitializer
{
public:
	TCPConnInitializer(TCPServer::Impl & impl, const int socket, const char * peerIP, const quint16 peerPort,
		TLSStream * tlsStream, uint tlsThreadId)
	:
		impl(impl), socket(socket), peerIP(peerIP), peerPort(peerPort), tlsStream(tlsStream), tlsThreadId(tlsThreadId)
	{}

	TCPServer::Impl & impl;
	const int socket;
	const QByteArray peerIP;
	const quint16 peerPort;
	TLSStream * tlsStream;
	const uint tlsThreadId;
};

class TLSThread : public util::ThreadVerify
{
public:
	TLSThread(TCPServer::Impl & impl, uint no, uint total) :
		ThreadVerify(QString("TLSThread %1/%2").arg(no).arg(total), ThreadVerify::Worker),
		impl_(impl)
	{
	}

	~TLSThread()
	{
		stopVerifyThread();
	}

	void read(TCPConn * conn);
	void write(TCPConn * conn, const QByteArray & data, bool notifyFinished);
	void closeConn(TCPConn * conn, TCPConn::CloseType type);
	void destroy(TCPConn * conn);

private:
	TCPServer::Impl & impl_;
};

class TCPServer::Impl : public util::ThreadVerify
{
public:
	Impl(TCPServer & parent, TLSCredentials * credentials, uint tlsThreadCount) :
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

	~Impl()
	{
		foreach (TLSThread * th, tlsThreads_) delete th;
		stopVerifyThread();
		delete sessions_;
		delete readWatcher_;
	}

	bool isRunning() const
	{
		return listenSock_ != -1;
	}

	virtual void deleteThreadData()
	{
		if (isRunning()) stop();
	}

	bool start(int listenSocket)
	{
		SyncedThreadCall<bool> stc(this);
		if (!stc.verify(&Impl::start, listenSocket)) return stc.retval();

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

	bool stop()
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

	void startReadWatcher(TCPConn * conn)
	{
		if (!verifyThreadCall(&Impl::startReadWatcher, conn)) return;

		const TCPConn::CloseType ct = conn->closeType_;
		if (ct & TCPConn::ReadClosed) {
			execCall(new Functor1<TCPConn, TCPConn::CloseType>(conn, &TCPConn::closed, ct));
			return;
		}
		ev_io_start(libEVLoop(), conn->readWatcher_);
	}

	void closeConn(TCPConn * conn, TCPConn::CloseType type)
	{
		if (!verifyThreadCall(&Impl::closeConn, conn, type)) return;

		// update state
		TCPConn::CloseType & ct = conn->closeType_;
		const TCPConn::CloseType oldCt = ct;
		ct = (TCPConn::CloseType)(ct | type);
		if (oldCt == ct) return;
		logDebug("socket %1 closed (%2 => %3)", conn->socket_, (int)oldCt, (int)ct);

		// calc changes
		const TCPConn::CloseType changed = (TCPConn::CloseType)(oldCt ^ ct);
		const bool readClosed  = changed & TCPConn::ReadClosed;
		const bool writeClosed = changed & TCPConn::WriteClosed;

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
		} else {
			if (writeClosed && !conn->writeBuf_.isEmpty()) {
				conn->closeAfterWriting_ = true;
			} else {
				shutdown(conn->socket_, readClosed && writeClosed ? SHUT_RDWR : (readClosed ? SHUT_RD : SHUT_WR));
			}
		}
	}

	const TCPConnInitializer * openConnection(const QByteArray & destIP, quint16 destPort,
		crypt::TLSCredentials * credentials)
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

		// bind to address and port
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
				new TCPConnInitializer(*this, sock, destIP, destPort,
					new TLSClient(*sessions_, *credentials), ++tlsConnId_ % tlsThreads_.size())
			:
				new TCPConnInitializer(*this, sock, destIP, destPort, 0, 0);

		logDebug("opened connection %1 to %2:%3", sock, ci->peerIP, ci->peerPort);
		return ci;
	}

	void destroy(TCPConn * conn)
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

	void deleteConn(TCPConn * conn)
	{
		if (!verifyThreadCall(&Impl::deleteConn, conn)) return;
		delete conn;
	}

	inline TCPServer & tcpServer() { return parent_; }

	inline void tlsWrite(TCPConn * conn, const QByteArray & data, bool notifyFinished) const
	{
		tlsThreads_[conn->tlsThreadId_]->write(conn, data, notifyFinished);
	}

	inline void tlsCloseConn(TCPConn * conn, TCPConn::CloseType type) const
	{
		tlsThreads_[conn->tlsThreadId_]->closeConn(conn, type);
	}

	static void readable(ev_loop * loop, ev_io * w, int /*revents*/)
	{
		TCPConn * conn = (TCPConn *)w->data;

		const ssize_t count = ::recv(conn->socket_, (char *)conn->readBuf_.constData(), conn->readBuf_.size(), 0);
		if (count > 0) {
			logTrace("read %1 raw bytes", (int)count);
			ev_io_stop(loop, w);
			if (!conn->tlsStream_) conn->newBytesAvailable();
			else                   conn->impl_.tlsThreads_[conn->tlsThreadId_]->read(conn);
			return;
		}

		if (count == 0) {
			logDebug("read channel closed on fd %1", conn->socket_);
			conn->impl_.closeConn(conn, TCPConn::ReadClosed);
			return;
		}

		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			logDebug("nothing to read on fd %1 (errno: %2)", conn->socket_, errno);
			return;
		}

		logInfo("read on fd %1 failed (errno: %2)", conn->socket_, errno);
		conn->impl_.closeConn(conn, TCPConn::HardClosed);
	}

	void writeToSocket(TCPConn * conn, const QByteArray & data, bool notifyFinished)
	{
		if (!verifyThreadCall(&Impl::writeToSocket, conn, data, notifyFinished)) return;

		conn->writeBuf_ += data;

		if (conn->writeBuf_.isEmpty()) {
			if (notifyFinished) execCall(new Functor0<TCPConn>(conn, &TCPConn::writeFinished));
			return;
		}

		if (conn->closeType_ & TCPConn::WriteClosed) {
			if (notifyFinished) execCall(new Functor1<TCPConn, TCPConn::CloseType>(conn, &TCPConn::closed, conn->closeType_));
			return;
		}

		if (notifyFinished) conn->notifyWrite_ = true;
		if (!ev_is_active(conn->writeWatcher_)) writeable(libEVLoop(), conn->writeWatcher_, 0);
	}

	static void writeable(ev_loop * loop, ev_io * w, int /*revents*/)
	{
		TCPConn * conn = (TCPConn *)w->data;

		QByteArray & buf = conn->writeBuf_;
		const int fd = conn->socket_;

		ssize_t count = ::send(fd, buf.constData(), buf.size(), 0);
		logTrace("wrote %1 / %2 bytes", (qint64)count, buf.size());
		if (count < buf.size()) {
			if (count < 0 && errno != EAGAIN && errno != EWOULDBLOCK && errno != ENOTCONN) {
				logDebug("write on fd %1 failed (errno: %2)", fd, errno);
				buf.clear();
				conn->impl_.closeConn(conn, errno == EPIPE ? TCPConn::WriteClosed : TCPConn::HardClosed);
				return;
			}
			if (count > 0) buf.remove(0, count);
			if (!ev_is_active(w)) ev_io_start(loop, w);
		} else {
			buf.clear();
			if (ev_is_active(w)) ev_io_stop(loop, w);
			if (conn->closeAfterWriting_ && conn->closeType_ != TCPConn::HardClosed)
				shutdown(conn->socket_, conn->closeType_ & TCPConn::ReadClosed ? SHUT_RDWR : SHUT_WR);
			if (conn->notifyWrite_) {
				conn->notifyWrite_ = false;
				if (conn->closeType_ & TCPConn::WriteClosed) {
					conn->impl_.execCall(new Functor1<TCPConn, TCPConn::CloseType>(conn, &TCPConn::closed, conn->closeType_));
				} else {
					conn->impl_.execCall(new Functor0<TCPConn>(conn, &TCPConn::writeFinished));
				}
			}
		}
	}

private:
	static void listenSocketReadable(ev_loop *, ev_io * w, int)
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
					new TCPConnInitializer(*impl,
						newSock, inet_ntop(AF_INET, &cliAddr.sin_addr, ipAddr, sizeof(ipAddr)), ntohs(cliAddr.sin_port),
						new TLSServer(*(impl->sessions_), *(impl->credentials_)),
						++impl->tlsConnId_ % impl->tlsThreads_.size())
				:
					new TCPConnInitializer(*impl,
						newSock, inet_ntop(AF_INET, &cliAddr.sin_addr, ipAddr, sizeof(ipAddr)), ntohs(cliAddr.sin_port),
						0, 0);
			logDebug("new connection (%1) from %2:%3", newSock, ci->peerIP, ci->peerPort);
			impl->parent_.newConnection(ci);
		}
	}

private:
	TCPServer & parent_;
	int listenSock_;
	ev_io * readWatcher_;
	TLSSessions * sessions_;
	TLSCredentials * credentials_;
	QVector<TLSThread *> tlsThreads_;
	uint tlsConnId_;
};

void TLSThread::read(TCPConn * conn)
{
	if (!verifyThreadCall(&TLSThread::read, conn)) return;

	QByteArray sendBack;
	QByteArray plain;
	if (!conn->tlsStream_->received(conn->readBuf_, plain, sendBack)) impl_.closeConn(conn, TCPConn::ReadWriteClosed);
	if (!sendBack.isEmpty()) impl_.writeToSocket(conn, sendBack, false);
	conn->readBuf_ = plain;
	conn->newBytesAvailable();
}

void TLSThread::write(TCPConn * conn, const QByteArray & data, bool notifyFinished)
{
	if (!verifyThreadCall(&TLSThread::write, conn, data, notifyFinished)) return;

	QByteArray enc;
	if (!conn->tlsStream_->send(data, enc)) impl_.closeConn(conn, TCPConn::ReadWriteClosed);
	impl_.writeToSocket(conn, enc, notifyFinished);
}

void TLSThread::closeConn(TCPConn * conn, TCPConn::CloseType type)
{
	if (!verifyThreadCall(&TLSThread::closeConn, conn, type)) return;
	impl_.closeConn(conn, type);
}

void TLSThread::destroy(TCPConn * conn)
{
	if (!verifyThreadCall(&TLSThread::destroy, conn)) return;
	impl_.deleteConn(conn);
}

TCPServer::TCPServer() :
	impl_(new Impl(*this, 0, 0))
{
}

TCPServer::TCPServer(TLSCredentials & credentials, uint tlsThreadCount) :
	impl_(new Impl(*this, &credentials, tlsThreadCount))
{
}

TCPServer::~TCPServer()
{
	delete impl_;
}

int TCPServer::openListenSocket(quint16 port, const QByteArray & ip)
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

bool TCPServer::start(int listenSocket)
{
	return impl_->start(listenSocket);
}

bool TCPServer::start(quint16 port, const QByteArray & ip)
{
	int listenSocket = openListenSocket(port, ip);
	if (listenSocket < 0) return false;
	return impl_->start(listenSocket);
}

bool TCPServer::stop()
{
	return impl_->stop();
}

bool TCPServer::isRunning() const
{
	return impl_->isRunning();
}

const TCPConnInitializer * TCPServer::openConnection(const QByteArray & destIP, quint16 destPort)
{
	return impl_->openConnection(destIP, destPort, 0);
}

const TCPConnInitializer * TCPServer::openConnection(const QByteArray & destIP, quint16 destPort,
	TLSCredentials & credentials)
{
	return impl_->openConnection(destIP, destPort, &credentials);
}

TCPConn::TCPConn(const TCPConnInitializer * init, uint readBufferSize) :
	impl_(init->impl),
	socket_(init->socket), peerIP_(init->peerIP), peerPort_(init->peerPort),
	readWatcher_(new ev_io),
	writeWatcher_(new ev_io),
	readBuf_(readBufferSize, '\0'),
	closeAfterWriting_(false), notifyWrite_(false), closeType_(NotClosed),
	tlsStream_(init->tlsStream), tlsThreadId_(init->tlsThreadId)
{
	delete init;
	ev_io_init(readWatcher_, &TCPServer::Impl::readable, socket_, EV_READ);
	readWatcher_->data = this;
	ev_io_init(writeWatcher_, &TCPServer::Impl::writeable, socket_, EV_WRITE);
	writeWatcher_->data = this;

	if (tlsStream_) {
		const QByteArray data = tlsStream_->initialSend();
		if (!data.isEmpty()) impl_.writeToSocket(this, data, false);
	}
}

TCPConn::~TCPConn()
{
	delete readWatcher_;
	delete writeWatcher_;
	delete tlsStream_;
}

void TCPConn::destroy()
{
	impl_.destroy(this);
}

QByteArray TCPConn::read()
{
	const QByteArray retval = readBuf_;
	readBuf_.clear();
	return retval;
}

void TCPConn::startReadWatcher()
{
	impl_.startReadWatcher(this);
}

void TCPConn::write(const QByteArray & data, bool notifyFinished)
{
	if (tlsStream_) impl_.tlsWrite(this, data, notifyFinished);
	else            impl_.writeToSocket(this, data, notifyFinished);
}

void TCPConn::close(CloseType type)
{
	if (tlsStream_) impl_.tlsCloseConn(this, type);
	else            impl_.closeConn(this, type);
}

const TCPConnInitializer * TCPConn::detachFromSocket()
{
	const TCPConnInitializer * rv = new TCPConnInitializer(impl_, socket_, peerIP_, peerPort_, tlsStream_, tlsThreadId_);
	socket_ = -1;
	closeType_ = HardClosed;
	tlsStream_ = 0;
	return rv;
}

void TCPConn::setNoDelay(bool noDelay)
{
	int on = noDelay ? 1 : 0;
	setsockopt(socket_, IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof(on));
}

TCPServer & TCPConn::server() const
{
	return impl_.tcpServer();
}

}}	// namespace

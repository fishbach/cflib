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
#include <cflib/libev/libev.h>
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

USE_LOG(LogCat::Network)

namespace cflib { namespace util {

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
		TLSStream * tlsStream)
	:
		impl(impl), socket(socket), peerIP(peerIP), peerPort(peerPort), tlsStream(tlsStream)
	{}

	TCPServer::Impl & impl;
	const int socket;
	const QByteArray peerIP;
	const quint16 peerPort;
	TLSStream * tlsStream;
};

class TCPServer::Impl : public util::ThreadVerify
{
public:
	Impl(TCPServer & parent, TLSCredentials * credentials) :
		ThreadVerify("TCPServer", ThreadVerify::Net),
		parent_(parent),
		listenSock_(-1),
		readWatcher_(new ev_io),
		sessions_(credentials ? new TLSSessions : 0),
		credentials_(credentials)
	{
	}

	~Impl()
	{
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
		ev_io_init(readWatcher_, &Impl::readable, listenSock_, EV_READ);
		readWatcher_->data = this;
		ev_io_start(libEVLoop(), readWatcher_);

		logInfo("server started");
		return true;
	}

	bool stop()
	{
		SyncedThreadCall<bool> stc(this);
		if (!stc.verify(&Impl::stop)) return stc.retval();

		if (isRunning()) {
			logWarn("server not running");
			return false;
		}

		ev_io_stop(libEVLoop(), readWatcher_);
		close(listenSock_);
		listenSock_ = -1;

		logInfo("server stopped");
		return true;
	}

	void startWatcher(TCPConn * conn)
	{
		if (!verifyThreadCall(&Impl::startWatcher, conn)) return;

		if (conn->socket_ == -1) {
			execCall(new Functor0<TCPConn>(conn, &TCPConn::closed));
			return;
		}
		ev_io_start(libEVLoop(), conn->readWatcher_);
	}

	void writeToSocket(TCPConn * conn, const QByteArray & data)
	{
		if (!verifyThreadCall(&Impl::writeToSocket, conn, data)) return;

		conn->writeBuf_ += data;
		TCPConn::writeable(libEVLoop(), conn->writeWatcher_, 0);
	}

	void closeSocket(TCPConn * conn)
	{
		if (!verifySyncedThreadCall(&Impl::closeSocket, conn)) return;

		if (conn->socket_ == -1) return;

		logDebug("socket %1 closed", conn->socket_);

		ev_io_stop(libEVLoop(), conn->writeWatcher_);
		ev_io_stop(libEVLoop(), conn->readWatcher_);
		close(conn->socket_);
		conn->socket_ = -1;
	}

	void closeNicely(TCPConn * conn)
	{
		if (!verifyThreadCall(&Impl::closeNicely, conn)) return;

		if (conn->socket_ == -1) return;

		logDebug("closing socket %1 nicely", conn->socket_);

		if (conn->writeBuf_.isEmpty()) {
			shutdown(conn->socket_, SHUT_RDWR);
		} else {
			conn->closeAfterWriting_ = true;
			shutdown(conn->socket_, SHUT_RD);
		}
	}

	const TCPConnInitializer * openConnection(const QByteArray & destIP, quint16 destPort,
		crypt::TLSCredentials * credentials)
	{
		// create socket
		int sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock < 0) return 0;

		if (!setNonBlocking(sock)) {
			close(sock);
			return 0;
		}

		// bind to address and port
		struct sockaddr_in destAddr;
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

		const TCPConnInitializer * ci = new TCPConnInitializer(*this, sock, destIP, destPort,
			credentials ? new TLSClient(*sessions_, *credentials) : 0);
		logDebug("opened connection %1 to %2:%3", sock, ci->peerIP, ci->peerPort);
		return ci;
	}

private:
	static void readable(ev_loop *, ev_io * w, int)
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
			const TCPConnInitializer * ci = new TCPConnInitializer(*impl,
				newSock, inet_ntop(AF_INET, &cliAddr.sin_addr, ipAddr, sizeof(ipAddr)), ntohs(cliAddr.sin_port),
				impl->credentials_ ? new TLSServer(*(impl->sessions_), *(impl->credentials_)) : 0);
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
};

TCPServer * TCPServer::instance_ = 0;

TCPServer::TCPServer() :
	impl_(new Impl(*this, 0))
{
	if (!instance_) instance_ = this;
}

TCPServer::TCPServer(TLSCredentials & credentials) :
	impl_(new Impl(*this, &credentials))
{
	if (!instance_) instance_ = this;
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

TCPConn::TCPConn(const TCPConnInitializer * init) :
	impl_(init->impl), socket_(init->socket), peerIP_(init->peerIP), peerPort_(init->peerPort),
	readWatcher_(new ev_io),
	writeWatcher_(new ev_io),
	readBuf_(0x10000, '\0'),
	closeAfterWriting_(false),
	tlsStream_(init->tlsStream)
{
	delete init;
	ev_io_init(readWatcher_, &TCPConn::readable, socket_, EV_READ);
	readWatcher_->data = this;
	ev_io_init(writeWatcher_, &TCPConn::writeable, socket_, EV_WRITE);
	writeWatcher_->data = this;

	if (tlsStream_) {
		const QByteArray data = tlsStream_->initialSend();
		if (!data.isEmpty()) impl_.writeToSocket(this, data);
	}
}

TCPConn::~TCPConn()
{
	impl_.closeSocket(this);
	delete readWatcher_;
	delete writeWatcher_;
	delete tlsStream_;
}

QByteArray TCPConn::read()
{
	QByteArray retval;
	if (socket_ == -1) return retval;

	forever {
		ssize_t count = ::recv(socket_, (char *)readBuf_.constData(), readBuf_.size(), 0);
		if (count == readBuf_.size() && retval.size() < 0x100000) {	// 1 mb
			retval.append(readBuf_.constData(), count);
			continue;
		}

		if (count == 0) {
			impl_.closeSocket(this);
			break;
		}

		if (count < 0) {
			if (errno != EAGAIN && errno != EWOULDBLOCK) {
				logDebug("read on fd %1 failed (errno: %2)", socket_, errno);
				impl_.closeSocket(this);
				break;
			}
		} else {
			retval.append(readBuf_.constData(), count);
		}

		logTrace("read %1 bytes", retval.size());
		break;
	}

	if (tlsStream_ && !retval.isEmpty()) {
		QByteArray plain;
		QByteArray sendBack;
		if (!tlsStream_->received(retval, plain, sendBack)) impl_.closeNicely(this);
		if (!sendBack.isEmpty()) impl_.writeToSocket(this, sendBack);
		return plain;
	}

	return retval;
}

void TCPConn::startWatcher()
{
	impl_.startWatcher(this);
}

void TCPConn::write(const QByteArray & data)
{
	if (tlsStream_) {
		QByteArray enc;
		if (!tlsStream_->send(data, enc)) impl_.closeNicely(this);
		impl_.writeToSocket(this, enc);
	} else {
		impl_.writeToSocket(this, data);
	}
}

void TCPConn::closeNicely()
{
	impl_.closeNicely(this);
}

void TCPConn::abortConnection()
{
	impl_.closeSocket(this);
}

const TCPConnInitializer * TCPConn::detachFromSocket()
{
	const TCPConnInitializer * rv = new TCPConnInitializer(impl_, socket_, peerIP_, peerPort_, tlsStream_);
	socket_ = -1;
	tlsStream_ = 0;
	return rv;
}

void TCPConn::setNoDelay(bool noDelay)
{
	int on = noDelay ? 1 : 0;
	setsockopt(socket_, IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof(on));
}

void TCPConn::readable(ev_loop * loop, ev_io * w, int)
{
	ev_io_stop(loop, w);
	((TCPConn *)w->data)->newBytesAvailable();
}

void TCPConn::writeable(ev_loop * loop, ev_io * w, int)
{
	TCPConn * conn = (TCPConn *)w->data;
	QByteArray & buf = conn->writeBuf_;
	const int fd = conn->socket_;
	if (buf.isEmpty() || fd == -1) return;

	ssize_t count = ::send(fd, buf.constData(), buf.size(), 0);
	logTrace("wrote %1 / %2 bytes", (qint64)count, buf.size());
	if (count < buf.size()) {
		if (count < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
			logDebug("write on fd %1 failed (errno: %2)", fd, errno);
			buf.clear();
			conn->impl_.closeSocket(conn);
			return;
		}
		if (count > 0) buf.remove(0, count);
		if (!ev_is_active(w)) ev_io_start(loop, w);
	} else {
		buf.clear();
		if (ev_is_active(w)) ev_io_stop(loop, w);
		if (conn->closeAfterWriting_) shutdown(conn->socket_, SHUT_WR);
	}
}

}}	// namespace

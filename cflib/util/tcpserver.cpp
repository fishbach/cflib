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

#include <cflib/libev/libev.h>
#include <cflib/util/log.h>
#include <cflib/util/threadverify.h>

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

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

class TCPServer::ConnInitializer
{
public:
	ConnInitializer(TCPServer::Impl & impl, const int socket, const char * peerIP, const quint16 peerPort) :
		impl(impl), socket(socket), peerIP(peerIP), peerPort(peerPort) {}

	TCPServer::Impl & impl;
	const int socket;
	const QByteArray peerIP;
	const quint16 peerPort;
};

class TCPServer::Impl : public util::ThreadVerify
{
public:
	Impl(TCPServer & parent) :
		ThreadVerify("TCPServer", ThreadVerify::Net),
		parent_(parent),
		listenSock_(-1),
		readWatcher_(new ev_io)
	{
	}

	~Impl()
	{
		stopVerifyThread();
		delete readWatcher_;
	}

	virtual void deleteThreadData()
	{
		if (listenSock_ != -1) {
			ev_io_stop(libEVLoop(), readWatcher_);
			close(listenSock_);
		}
	}

	bool start(quint16 port, const QByteArray & ip)
	{
		SyncedThreadCall<bool> stc(this);
		if (!stc.verify(&Impl::start, port, ip)) return stc.retval();

		logFunctionTrace

		if (listenSock_ >= 0) {
			logWarn("server already listening");
			return false;
		}

		// create socket
		listenSock_ = socket(AF_INET, SOCK_STREAM, 0);
		if (listenSock_ < 0) {
			logWarn("cannot create TCP socket (errno: %1)", errno);
			return false;
		}
		if (!setNonBlocking(listenSock_)) return false;
		{ int on = 1; setsockopt(listenSock_, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof(on)); }

		// bind to address and port
		struct sockaddr_in servAddr;
		servAddr.sin_family = AF_INET;
		if (inet_aton(ip.constData(), &servAddr.sin_addr) == 0) {
			logWarn("no valid ip address: %1", ip);
			close(listenSock_);
			listenSock_ = -1;
			return false;
		}
		servAddr.sin_port = htons(port);
		if (bind(listenSock_, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
			logWarn("cannot bind to %1:%2 (errno: %3)", ip, port, errno);
			close(listenSock_);
			listenSock_ = -1;
			return false;
		}

		// start listening
		if (listen(listenSock_, 1024) < 0) {
			logWarn("cannot listen on fd %1 (errno: %4)", listenSock_, errno);
			close(listenSock_);
			listenSock_ = -1;
			return false;
		}

		// watching for incoming activity
		ev_io_init(readWatcher_, &Impl::readable, listenSock_, EV_READ);
		readWatcher_->data = this;
		ev_io_start(libEVLoop(), readWatcher_);

		logInfo("listening on %1:%2", ip, port);
		return true;
	}

	void startReadWatcher(TCPConn * conn)
	{
		if (!verifyThreadCall(&Impl::startReadWatcher, conn)) return;

		ev_io_start(libEVLoop(), conn->readWatcher_);
	}

	void writeToSocket(TCPConn * conn, const QByteArray & data)
	{
		if (!verifyThreadCall(&Impl::writeToSocket, conn, data)) return;

		conn->writeBuf_ += data;
		TCPConn::writeable(libEVLoop(), conn->writeWatcher_, 0);
	}

	void closeSocket(TCPConn * conn, bool informApi)
	{
		if (!verifyThreadCall(&Impl::closeSocket, conn, informApi)) return;

		if (conn->socket_ == -1) return;

		logFunctionTraceParam("closing socket %1", conn->socket_);

		ev_io_stop(libEVLoop(), conn->writeWatcher_);
		ev_io_stop(libEVLoop(), conn->readWatcher_);
		close(conn->socket_);
		conn->socket_ = -1;

		if (informApi) conn->closed();
	}

	void closeSocketSync(TCPConn * conn)
	{
		if (!verifySyncedThreadCall(&Impl::closeSocketSync, conn)) return;
		closeSocket(conn, false);
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

			const ConnInitializer * ci = new ConnInitializer(*impl,
				newSock, inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port));
			logDebug("new connection from %1:%2", ci->peerIP, ci->peerPort);
			impl->parent_.newConnection(ci);
		}
	}

private:
	TCPServer & parent_;
	int listenSock_;
	ev_io * readWatcher_;
};

TCPServer::TCPServer() :
	impl_(new Impl(*this))
{
}

TCPServer::~TCPServer()
{
	delete impl_;
}

bool TCPServer::start(quint16 port, const QByteArray & ip)
{
	return impl_->start(port, ip);
}

TCPConn::TCPConn(const TCPServer::ConnInitializer * init) :
	impl_(init->impl), socket_(init->socket), peerIP_(init->peerIP), peerPort_(init->peerPort),
	readWatcher_(new ev_io),
	writeWatcher_(new ev_io),
	readBuf_(0x10000, '\0')
{
	delete init;
	ev_io_init(readWatcher_, &TCPConn::readable, socket_, EV_READ);
	readWatcher_->data = this;
	ev_io_init(writeWatcher_, &TCPConn::writeable, socket_, EV_WRITE);
	writeWatcher_->data = this;
}

TCPConn::~TCPConn()
{
	if (socket_ != -1) impl_.closeSocketSync(this);
	delete readWatcher_;
	delete writeWatcher_;
}

QByteArray TCPConn::read()
{
	QByteArray retval;
	if (socket_ == -1) return retval;

	forever {
		ssize_t count = ::read(socket_, (void *)readBuf_.constData(), readBuf_.size());
		if (count == readBuf_.size() && retval.size() < 0x40000000) {	// 1 mb
			retval.append(readBuf_.constData(), count);
			continue;
		}

		if (count == 0) {
			impl_.closeSocket(this, true);
			return retval;
		}

		if (count < 0) {
			if (errno != EAGAIN && errno != EWOULDBLOCK) {
				logDebug("read on fd %1 failed (errno: %2)", socket_, errno);
				impl_.closeSocket(this, true);
				return retval;
			}
		} else {
			retval.append(readBuf_.constData(), count);
		}

		logTrace("read %1 bytes", retval.size());
		return retval;
	}
}

void TCPConn::startReadWatcher()
{
	impl_.startReadWatcher(this);
}

void TCPConn::write(const QByteArray & data)
{
	impl_.writeToSocket(this, data);
}

void TCPConn::close()
{
	shutdown(socket_, SHUT_RDWR);
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

	ssize_t count = ::write(fd, buf.constData(), buf.size());
	logTrace("wrote %1 / %2 bytes", (qint64)count, buf.size());
	if (count < buf.size()) {
		if (count < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
			logDebug("write on fd %1 failed (errno: %2)", fd, errno);
			buf.clear();
			conn->impl_.closeSocket(conn, true);
			return;
		}
		if (count > 0) buf.remove(0, count);
		if (!ev_is_active(w)) ev_io_start(loop, w);
	} else {
		buf.clear();
		if (ev_is_active(w)) ev_io_stop(loop, w);
	}
}

}}	// namespace

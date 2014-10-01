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

#include <cflib/util/log.h>
#include <cflib/util/threadverify.h>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

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
		logWarn("cannot set fd %1 non blocking", fd);
		return false;
	}
#endif
	return true;
}

}

class TCPServer::Impl : public util::ThreadVerify
{
public:
	Impl(TCPServer & parent) :
		ThreadVerify("TCPServer", true),
		parent_(parent),
		listenSock_(-1)
	{
	}

	virtual ~Impl()
	{
		stopVerifyThread();
	}

	bool start(quint16 port, const QByteArray & ip)
	{
		SyncedThreadCall<bool> stc(this);
		if (!stc.verify(&Impl::start, port, ip)) return stc.retval();

		if (listenSock_ >= 0) {
			logWarn("server already listening");
			return false;
		}

		// create socket
		listenSock_ = socket(AF_INET, SOCK_STREAM, 0);
		if (listenSock_ < 0) {
			logWarn("cannot create TCP socket");
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
			logWarn("cannot bind to %1:%2", ip, port);
			close(listenSock_);
			listenSock_ = -1;
			return false;
		}

		// start listening
		if (listen(listenSock_, 1024) < 0) {
			logWarn("cannot listen on fd %1", listenSock_);
			close(listenSock_);
			listenSock_ = -1;
			return false;
		}

		// watching for incoming activity
		ev_io_init(&readWatcher_, Impl::readable, listenSock_, EV_READ);
		readWatcher_.data = this;
		ev_io_start(libEVLoop(), &readWatcher_);

		logInfo("listening on %1:%2", ip, port);
		return true;
	}

private:
	static void readable(ev_loop *, ev_io * w, int)
	{
		Impl * impl = (Impl *)w->data;
		forever {
			struct sockaddr_in cliAddr;
			socklen_t len = sizeof(cliAddr);
			int newSock = accept(impl->listenSock_, (struct sockaddr *)&cliAddr, &len);
			if (newSock < 0) break;

			setNonBlocking(newSock);

			QByteArray peerIP = inet_ntoa(cliAddr.sin_addr);
			quint16 peerPort = ntohs(cliAddr.sin_port);
			logDebug("new connection from %1:%2", peerIP, peerPort);
			impl->parent_.newConnection(newSock, peerIP, peerPort);
		}
	}

private:
	TCPServer & parent_;
	int listenSock_;
	ev_io readWatcher_;
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

}}	// namespace

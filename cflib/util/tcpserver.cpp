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

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

USE_LOG(LogCat::Network)

namespace cflib { namespace util {

TCPServer::TCPServer() :
	networkThread_("Network", true),
	listenSock_(-1)
{
}

TCPServer::~TCPServer()
{
	networkThread_.stopVerifyThread();
}

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

bool TCPServer::start(quint16 port, const QByteArray & ip)
{
	// create socket
	listenSock_ = ::socket(AF_INET, SOCK_STREAM, 0);
	if (listenSock_ < 0) {
		logWarn("cannot create TCP socket");
		return false;
	}
	if (!setNonBlocking(listenSock_)) return false;
	{ int on = 1; setsockopt(listenSock_, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof(on)); }

	// bind to address and port
	struct sockaddr_in srcAddr;
	srcAddr.sin_family = AF_INET;
	if (inet_aton(ip.constData(), &srcAddr.sin_addr) == 0) {
		logWarn("no valid ip address: %1", ip);
		close(listenSock_);
		listenSock_ = -1;
		return false;
	}
	srcAddr.sin_port = htons(port);
	if (bind(listenSock_, (struct sockaddr *)&srcAddr, sizeof(srcAddr)) < 0) {
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

	int newsockfd;
	socklen_t clilen;
	char buffer[256];
	struct sockaddr_in cli_addr;
	int n;


	clilen = sizeof(cli_addr);
	newsockfd = accept(listenSock_, (struct sockaddr *)&cli_addr, &clilen);

	if (newsockfd < 0) ;//error("ERROR on accept");

	bzero(buffer,256);
	n = read(newsockfd,buffer,255);
	if (n < 0) ;//error("ERROR reading from socket");
	printf("Here is the message: %s\n",buffer);
	n = write(newsockfd,"I got your message",18);
	if (n < 0) ;//error("ERROR writing to socket");
	close(newsockfd);


}

}}	// namespace

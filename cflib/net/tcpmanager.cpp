/* Copyright (C) 2013-2016 Christian Fischbach <cf@cflib.de>
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

#include "tcpmanager.h"

#include <cflib/net/impl/tcpmanagerimpl.h>

namespace cflib { namespace net {

TCPManager::TCPManager(uint tlsThreadCount, util::ThreadVerify * other) :
	impl_(other ?
		new impl::TCPManagerImpl(*this, tlsThreadCount, other) :
		new impl::TCPManagerImpl(*this, tlsThreadCount))
{
}

TCPManager::~TCPManager()
{
	delete impl_;
}

void TCPManager::stop()
{
	impl_->stop();
}

bool TCPManager::isRunning() const
{
	return impl_->isRunning();
}

TCPConnData * TCPManager::openConnection(const QByteArray & destAddress, quint16 destPort, bool preferIPv6)
{
	return impl_->openConnection(destAddress, destPort, QByteArray(), 0, 0, preferIPv6);
}

TCPConnData * TCPManager::openConnection(const QByteArray & destAddress, quint16 destPort,
	crypt::TLSCredentials & credentials, bool preferIPv6)
{
	return impl_->openConnection(destAddress, destPort, QByteArray(), 0, &credentials, preferIPv6);
}

TCPConnData * TCPManager::openConnection(const QByteArray & destAddress, quint16 destPort,
	const QByteArray & sourceIP, quint16 sourcePort, bool preferIPv6)
{
	return impl_->openConnection(destAddress, destPort, sourceIP, sourcePort, 0, preferIPv6);
}

TCPConnData * TCPManager::openConnection(const QByteArray & destAddress, quint16 destPort,
	const QByteArray & sourceIP, quint16 sourcePort,
	crypt::TLSCredentials & credentials, bool preferIPv6)
{
	return impl_->openConnection(destAddress, destPort, sourceIP, sourcePort, &credentials, preferIPv6);
}

int TCPManager::openListenSocket(const QByteArray & ip, quint16 port)
{
	return impl::TCPManagerImpl::openListenSocket(ip, port);
}

bool TCPManager::start(int listenSocket)
{
	return impl_->start(listenSocket, 0);
}

bool TCPManager::start(int listenSocket, crypt::TLSCredentials & credentials)
{
	return impl_->start(listenSocket, &credentials);
}

util::ThreadVerify * TCPManager::networkThread()
{
	return impl_;
}

void TCPManager::newConnection(TCPConnData * data)
{
	impl_->deleteOnFinish(data);
}

}}	// namespace

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

#include "tcpmanager.h"

#include <cflib/net/impl/tcpmanagerimpl.h>

namespace cflib { namespace net {

TCPManager::TCPManager() :
	impl_(new Impl(*this, 0, 0))
{
}

TCPManager::TCPManager(crypt::TLSCredentials & credentials, uint tlsThreadCount) :
	impl_(new Impl(*this, &credentials, tlsThreadCount))
{
}

TCPManager::~TCPManager()
{
	delete impl_;
}

bool TCPManager::stop()
{
	return impl_->stop();
}

bool TCPManager::isRunning() const
{
	return impl_->isRunning();
}

const TCPConnInitializer * TCPManager::openConnection(const QByteArray & destIP, quint16 destPort)
{
	return impl_->openConnection(destIP, destPort, QByteArray(), 0, 0);
}

const TCPConnInitializer * TCPManager::openConnection(const QByteArray & destIP, quint16 destPort,
	crypt::TLSCredentials & credentials)
{
	return impl_->openConnection(destIP, destPort, QByteArray(), 0, &credentials);
}

const TCPConnInitializer * TCPManager::openConnection(const QByteArray & destIP, quint16 destPort,
	const QByteArray & sourceIP, quint16 sourcePort)
{
	return impl_->openConnection(destIP, destPort, sourceIP, sourcePort, 0);
}

const TCPConnInitializer * TCPManager::openConnection(const QByteArray & destIP, quint16 destPort,
	const QByteArray & sourceIP, quint16 sourcePort,
	crypt::TLSCredentials & credentials)
{
	return impl_->openConnection(destIP, destPort, sourceIP, sourcePort, &credentials);
}

int TCPManager::openListenSocket(const QByteArray & ip, quint16 port)
{
	return Impl::openListenSocket(ip, port);
}

bool TCPManager::start(int listenSocket)
{
	return impl_->start(listenSocket);
}

void TCPManager::newConnection(const TCPConnInitializer *)
{
}

}}	// namespace

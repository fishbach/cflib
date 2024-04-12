/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
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
    const QByteArray & sourceIP, quint16 sourcePort, bool preferIPv6)
{
    return impl_->openConnection(destAddress, destPort, sourceIP, sourcePort, 0, preferIPv6);
}

TCPConnData * TCPManager::openTLSConnection(const QByteArray & destAddress, quint16 destPort, bool preferIPv6)
{
    return impl_->openConnection(destAddress, destPort, QByteArray(), 0, &impl_->clientCredentials, preferIPv6);
}

TCPConnData * TCPManager::openTLSConnection(const QByteArray & destAddress, quint16 destPort,
    const QByteArray & sourceIP, quint16 sourcePort, bool preferIPv6)
{
    return impl_->openConnection(destAddress, destPort, sourceIP, sourcePort, &impl_->clientCredentials, preferIPv6);
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

crypt::TLSCredentials & TCPManager::clientCredentials()
{
    return impl_->clientCredentials;
}

void TCPManager::newConnection(TCPConnData * data)
{
    impl_->deleteOnFinish(data);
}

}}    // namespace

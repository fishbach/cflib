/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <QtCore>

namespace cflib { namespace crypt { class TLSCredentials; }}
namespace cflib { namespace util  { class ThreadVerify; }}

namespace cflib { namespace net {

class TCPConnData;
namespace impl { class TCPManagerImpl; }

class TCPManager
{
    Q_DISABLE_COPY(TCPManager)
public:
    // tlsThreadCount must be set > 0 when TLS is used
    TCPManager(uint tlsThreadCount = 0, util::ThreadVerify * other = 0);
    virtual ~TCPManager();

    bool start(const QByteArray & ip, quint16 port) { return start(openListenSocket(ip, port)); }
    bool start(const QByteArray & ip, quint16 port, crypt::TLSCredentials & credentials) {
        return start(openListenSocket(ip, port), credentials); }
    void stop();
    bool isRunning() const;

    TCPConnData * openConnection(const QByteArray & destAddress, quint16 destPort, bool preferIPv6 = false);
    TCPConnData * openConnection(const QByteArray & destAddress, quint16 destPort,
        const QByteArray & sourceIP, quint16 sourcePort, bool preferIPv6 = false);

    TCPConnData * openTLSConnection(const QByteArray & destAddress, quint16 destPort, bool preferIPv6 = false);
    TCPConnData * openTLSConnection(const QByteArray & destAddress, quint16 destPort,
        const QByteArray & sourceIP, quint16 sourcePort, bool preferIPv6 = false);

    static int openListenSocket(const QByteArray & ip, quint16 port);
    bool start(int listenSocket);
    bool start(int listenSocket, crypt::TLSCredentials & credentials);

    util::ThreadVerify * networkThread();

    crypt::TLSCredentials & clientCredentials();

protected:
    virtual void newConnection(TCPConnData * data);

private:
    impl::TCPManagerImpl * impl_;
    friend class impl::TCPManagerImpl;
};

}}    // namespace

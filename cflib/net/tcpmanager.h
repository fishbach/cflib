/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>

namespace cflib::crypt { class TLSCredentials; }
namespace cflib::util { class ThreadVerify; }

namespace cflib::net {

class TCPConnData;
namespace impl { class TCPManagerImpl; }

class TCPManager
{
    CF_DISABLE_COPY(TCPManager)
public:
    // tlsThreadCount must be set > 0 when TLS is used
    TCPManager(uint tlsThreadCount = 0, util::ThreadVerify * other = 0);
    virtual ~TCPManager();

    bool start(const ByteArray & ip, uint16 port) { return start(openListenSocket(ip, port)); }
    bool start(const ByteArray & ip, uint16 port, crypt::TLSCredentials & credentials) {
        return start(openListenSocket(ip, port), credentials); }
    void stop();
    bool isRunning() const;

    TCPConnData * openConnection(const ByteArray & destAddress, uint16 destPort, bool preferIPv6 = false);
    TCPConnData * openConnection(const ByteArray & destAddress, uint16 destPort,
        const ByteArray & sourceIP, uint16 sourcePort, bool preferIPv6 = false);

    TCPConnData * openTLSConnection(const ByteArray & destAddress, uint16 destPort, bool preferIPv6 = false);
    TCPConnData * openTLSConnection(const ByteArray & destAddress, uint16 destPort,
        const ByteArray & sourceIP, uint16 sourcePort, bool preferIPv6 = false);

    static int openListenSocket(const ByteArray & ip, uint16 port);
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

} // namespace

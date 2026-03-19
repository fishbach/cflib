/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/net/tcpconn.h>

struct ev_io;

namespace cflib::crypt { class TLSStream; }

namespace cflib::net {

namespace impl { class TCPManagerImpl; }

class TCPConnData
{
    CF_DISABLE_COPY(TCPConnData)
public:
    TCPConnData(impl::TCPManagerImpl & impl,
        int socket, const char * peerIP, cfuint16 peerPort,
        crypt::TLSStream * tlsStream, uint tlsThreadId);
    ~TCPConnData();

    void callClosed();

public:
    impl::TCPManagerImpl & impl;
    TCPConn * conn;

    // connection
    const int socket;
    const ByteArray peerIP;
    const cfuint16 peerPort;

    // TLS handling
    crypt::TLSStream * const tlsStream;
    const uint tlsThreadId;

    // state
    ev_io * readWatcher;
    ev_io * writeWatcher;
    ByteArray readBuf;
    ByteArray readData;
    ByteArray writeBuf;
    bool notifySomeBytesWritten;
    bool closeAfterWriting;
    bool deleteAfterWriting;
    bool notifyWrite;
    volatile TCPConn::CloseType closeType;
    TCPConn::CloseType lastInformedCloseType;
};

} // namespace

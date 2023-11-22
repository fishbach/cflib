/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "tcpconndata.h"

#include <cflib/crypt/tlsstream.h>
#include <cflib/net/impl/tcpmanagerimpl.h>
#include <cflib/util/libev.h>
#include <cflib/util/log.h>

USE_LOG(LogCat::Network)

namespace cflib { namespace net {

TCPConnData::TCPConnData(impl::TCPManagerImpl & impl,
    int socket, const char * peerIP, quint16 peerPort,
    crypt::TLSStream * tlsStream, uint tlsThreadId)
:
    impl(impl), conn(0),
    socket(socket), peerIP(peerIP), peerPort(peerPort),
    tlsStream(tlsStream), tlsThreadId(tlsThreadId),
    readWatcher(new ev_io), writeWatcher(new ev_io),
    notifySomeBytesWritten(false), closeAfterWriting(false), deleteAfterWriting(false), notifyWrite(false),
    closeType(TCPConn::NotClosed), lastInformedCloseType(TCPConn::NotClosed)
{
    ev_io_init(readWatcher, &impl::TCPManagerImpl::readable, socket, EV_READ);
    readWatcher->data = this;
    ev_io_init(writeWatcher, &impl::TCPManagerImpl::writeable, socket, EV_WRITE);
    writeWatcher->data = this;

    if (tlsStream) {
        const QByteArray data = tlsStream->initialSend();
        if (!data.isEmpty()) impl.writeToSocket(this, data, false);
    }
}

TCPConnData::~TCPConnData()
{
    delete tlsStream;
    delete readWatcher;
    delete writeWatcher;
    logTrace("~TCPConnData()");
}

void TCPConnData::callClosed()
{
    conn->closed(closeType);
}

}}    // namespace

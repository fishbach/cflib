/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "tlsthread.h"

#include <cflib/crypt/tlsstream.h>
#include <cflib/net/impl/tcpconndata.h>
#include <cflib/net/impl/tcpmanagerimpl.h>

namespace cflib { namespace net { namespace impl {

TLSThread::TLSThread(TCPManagerImpl & impl, uint no, uint total) :
    ThreadVerify(QString("TLSThread %1/%2").arg(no).arg(total), ThreadVerify::Worker),
    impl_(impl)
{
}

TLSThread::~TLSThread()
{
    stopVerifyThread();
}

void TLSThread::startReadWatcher(TCPConnData * conn)
{
    if (!verifyThreadCall(&TLSThread::startReadWatcher, conn)) return;
    impl_.startReadWatcher(conn);
}

void TLSThread::read(TCPConnData * conn)
{
    if (!verifyThreadCall(&TLSThread::read, conn)) return;

    QByteArray sendBack;
    QByteArray plain;
    bool ok = conn->tlsStream->received(conn->readData, plain, sendBack);
    if (!sendBack.isEmpty()) impl_.writeToSocket(conn, sendBack, false);

    if (plain.isEmpty()) {
        conn->readData.resize(0);
        if (ok) impl_.startReadWatcher(conn);
        else    impl_.closeConn(conn, TCPConn::ReadWriteClosed, true);
    } else {
        conn->readData = plain;
        conn->conn->newBytesAvailable();
        if (!ok) impl_.closeConn(conn, TCPConn::ReadWriteClosed, false);
    }
}

void TLSThread::write(TCPConnData * conn, const QByteArray & data, bool notifyFinished)
{
    if (!verifyThreadCall(&TLSThread::write, conn, data, notifyFinished)) return;

    QByteArray enc;
    if (!conn->tlsStream->send(data, enc)) {
        impl_.closeConn(conn, TCPConn::ReadWriteClosed, notifyFinished);
    } else {
        impl_.writeToSocket(conn, enc, notifyFinished);
    }
}

void TLSThread::closeConn(TCPConnData * conn, TCPConn::CloseType type, bool notifyClose)
{
    if (!verifyThreadCall(&TLSThread::closeConn, conn, type, notifyClose)) return;
    impl_.closeConn(conn, type, notifyClose);
}

void TLSThread::deleteOnFinish(TCPConnData * conn)
{
    if (!verifyThreadCall(&TLSThread::deleteOnFinish, conn)) return;
    impl_.deleteOnFinish(conn);
}

void TLSThread::callClosed(TCPConnData * conn)
{
    if (!verifyThreadCall(&TLSThread::callClosed, conn)) return;
    conn->callClosed();
}

}}}    // namespace

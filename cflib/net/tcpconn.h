/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <QtCore>

namespace cflib { namespace net {

class TCPConnData;
class TCPManager;
namespace impl { class TCPManagerImpl; }
namespace impl { class TLSThread; }

class TCPConn
{
    Q_DISABLE_COPY(TCPConn)
public:
    enum CloseType {
        NotClosed       = 0,
        ReadClosed      = 1,
        WriteClosed     = 2,
        ReadWriteClosed = 3,
        HardClosed      = 7
    };

public:
    TCPConn(TCPConnData * data, uint readBufferSize = 0x10000 /* 64kb */, bool notifySomeBytesWritten = false);

    // When destroying TCPConn object, make sure no callback is still active:
    // every call to startReadWatcher() will result in exactly one call of "newBytesAvailable" or "closed"
    // every call to write(data, true) will result in exactly one call of "writeFinished" or "closed"
    // exception: closed is only called once for each close state
    // example:
    //   startReadWatcher() -> peer closed our read channel -> closed(ReadClosed)
    //   if startReadWatcher() is called again, no callback will happen
    // destructor returns immediately, but write data will be send completely
    // If this is not desired, call close(HardClosed) before destruction.
    virtual ~TCPConn();

    // returns all available bytes
    QByteArray read();

    // buffers any amount of bytes passed
    // if notifyFinished == true, function writeFinished will be called when all bytes got written.
    void write(const QByteArray & data, bool notifyFinished = false);

    // closes the socket
    // - WriteClosed closes the write channel after all bytes have been written
    // - HardClosed may abort pending writes
    // - If readWatcher was active, "closed" will be called even when notifyClose == false.
    // - If notifyClose == true it is guaranteed that "closed" will be called.
    // on TCP level:
    // - WriteClosed sends FIN to peer
    // - ReadClosed does not inform the sender in any way and just releases some system resources (!)
    // - HardClosed sends RST to peer
    void close(CloseType type = ReadWriteClosed, bool notifyClose = false);

    // has to be called repeatedly to get informed via function newBytesAvailable
    void startReadWatcher();

    CloseType isClosed() const;
    QByteArray peerIP() const;
    quint16 peerPort() const;

    TCPConnData * detach();

    // sets TCP_NODELAY flag on socket
    void setNoDelay(bool noDelay);

    TCPManager & manager() const;

protected:
    virtual void newBytesAvailable() {}
    virtual void closed(CloseType type) { Q_UNUSED(type); }
    virtual void writeFinished() {}
    virtual void someBytesWritten(quint64 count) { Q_UNUSED(count) }

private:
    TCPConnData * data_;
    friend class TCPConnData;
    friend class impl::TCPManagerImpl;
    friend class impl::TLSThread;
};

}}    // namespace

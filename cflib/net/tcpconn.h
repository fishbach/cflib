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

#pragma once

#include <QtCore>

struct ev_io;

namespace cflib { namespace crypt { class TLSStream; }}

namespace cflib { namespace net {

class TCPConnInitializer;
class TCPManager;

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
	TCPConn(const TCPConnInitializer * init, uint readBufferSize = 0x100000 /* 1mb */);
	virtual ~TCPConn();

	// returns all available bytes
	QByteArray read();

	// buffers any amount of bytes passed
	// if notifyFinished == true, function writeFinished will be called when all bytes got written.
	void write(const QByteArray & data, bool notifyFinished = false);

	// closes the socket
	// on TCP level:
	// - WriteClosed sends FIN to peer
	// - ReadClosed does not inform the sender in any way (!)
	void close(CloseType type = ReadWriteClosed);

	// has to be called repeatedly to get informed via function newBytesAvailable
	void startReadWatcher();

	CloseType isClosed() const { return closeType_; }

	QByteArray peerIP() const { return peerIP_; }
	quint16 peerPort() const { return peerPort_; }

	const TCPConnInitializer * detachFromSocket();

	// sets TCP_NODELAY flag on socket
	void setNoDelay(bool noDelay);

	// deletes all ressources and this
	void destroy();

	TCPManager & manager() const { return mgr_; }

protected:
	virtual void newBytesAvailable() = 0;
	virtual void closed(CloseType type) = 0;
	virtual void writeFinished() {}

private:
	TCPManager & mgr_;

	// connection
	int socket_;
	const QByteArray peerIP_;
	const quint16 peerPort_;

	// state
	ev_io * readWatcher_;
	ev_io * writeWatcher_;
	QByteArray readBuf_;
	QByteArray readData_;
	QByteArray writeBuf_;
	bool closeAfterWriting_;
	bool notifyWrite_;
	CloseType closeType_;

	// TLS handling
	crypt::TLSStream * tlsStream_;
	const uint tlsThreadId_;

	friend class TCPManager;
	friend class TLSThread;
};

}}	// namespace

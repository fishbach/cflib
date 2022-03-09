/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/net/tcpconn.h>

struct ev_io;

namespace cflib { namespace crypt { class TLSStream; }}

namespace cflib { namespace net {

namespace impl { class TCPManagerImpl; }

class TCPConnData
{
	Q_DISABLE_COPY(TCPConnData)
public:
	TCPConnData(impl::TCPManagerImpl & impl,
		int socket, const char * peerIP, quint16 peerPort,
		crypt::TLSStream * tlsStream, uint tlsThreadId);
	~TCPConnData();

	void callClosed();

public:
	impl::TCPManagerImpl & impl;
	TCPConn * conn;

	// connection
	const int socket;
	const QByteArray peerIP;
	const quint16 peerPort;

	// TLS handling
	crypt::TLSStream * const tlsStream;
	const uint tlsThreadId;

	// state
	ev_io * readWatcher;
	ev_io * writeWatcher;
	QByteArray readBuf;
	QByteArray readData;
	QByteArray writeBuf;
	bool notifySomeBytesWritten;
	bool closeAfterWriting;
	bool deleteAfterWriting;
	bool notifyWrite;
	volatile TCPConn::CloseType closeType;
	TCPConn::CloseType lastInformedCloseType;
};

}}	// namespace

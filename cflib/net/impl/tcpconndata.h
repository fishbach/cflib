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
	bool closeAfterWriting;
	bool deleteAfterWriting;
	bool notifyWrite;
	volatile TCPConn::CloseType closeType;
};

}}	// namespace

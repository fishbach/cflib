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

#include "tcpconndata.h"

#include <cflib/crypt/tlsstream.h>
#include <cflib/net/impl/tcpmanagerimpl.h>

namespace cflib { namespace net {

TCPConnData::TCPConnData(TCPManagerImpl & impl,
	int socket, const char * peerIP, quint16 peerPort,
	crypt::TLSStream * tlsStream, uint tlsThreadId)
:
	impl(impl), conn(0),
	socket(socket), peerIP(peerIP), peerPort(peerPort),
	tlsStream(tlsStream), tlsThreadId(tlsThreadId),
	closeAfterWriting(false), notifyWrite(false), closeType(TCPConn::NotClosed)
{
	ev_io_init(&readWatcher, &TCPManagerImpl::readable, socket, EV_READ);
	readWatcher.data = this;
	ev_io_init(&writeWatcher, &TCPManagerImpl::writeable, socket, EV_WRITE);
	writeWatcher.data = this;

	if (tlsStream) {
		const QByteArray data = tlsStream->initialSend();
		if (!data.isEmpty()) impl.writeToSocket(this, data, false);
	}
}

TCPConnData::~TCPConnData()
{
	delete tlsStream;
}

}}	// namespace

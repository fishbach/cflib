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

void TLSThread::read(TCPConnData * conn)
{
	if (!verifyThreadCall(&TLSThread::read, conn)) return;

	QByteArray sendBack;
	QByteArray plain;
	if (!conn->tlsStream->received(conn->readData, plain, sendBack)) impl_.closeConn(conn, TCPConn::ReadWriteClosed);
	if (!sendBack.isEmpty()) impl_.writeToSocket(conn, sendBack, false);
	if (plain.isEmpty()) {
		conn->readData.resize(0);
		impl_.startReadWatcher(conn);
	} else {
		conn->readData = plain;
		conn->conn->newBytesAvailable();
	}
}

void TLSThread::write(TCPConnData * conn, const QByteArray & data, bool notifyFinished)
{
	if (!verifyThreadCall(&TLSThread::write, conn, data, notifyFinished)) return;

	QByteArray enc;
	if (!conn->tlsStream->send(data, enc)) impl_.closeConn(conn, TCPConn::ReadWriteClosed);
	impl_.writeToSocket(conn, enc, notifyFinished);
}

void TLSThread::closeConn(TCPConnData * conn, TCPConn::CloseType type)
{
	if (!verifyThreadCall(&TLSThread::closeConn, conn, type)) return;
	impl_.closeConn(conn, type);
}

void TLSThread::destroy(TCPConnData * conn)
{
	if (!verifyThreadCall(&TLSThread::destroy, conn)) return;
	impl_.deleteConn(conn);
}

}}}	// namespace

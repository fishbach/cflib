/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
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

#include "tcpconn.h"

#include <cflib/net/impl/tcpconndata.h>
#include <cflib/net/impl/tcpmanagerimpl.h>
#include <cflib/util/log.h>

USE_LOG(LogCat::Network)

namespace cflib { namespace net {

TCPConn::TCPConn(TCPConnData * data, uint readBufferSize, bool notifySomeBytesWritten) :
	data_(data)
{
	data_->conn = this;
	data_->readBuf = QByteArray(readBufferSize, '\0');
	data_->notifySomeBytesWritten = notifySomeBytesWritten;
}

TCPConn::~TCPConn()
{
	logFunctionTrace
	if (!data_) return;
	if (data_->tlsStream) data_->impl.tlsDeleteOnFinish(data_);
	else                  data_->impl.deleteOnFinish(data_);
}

QByteArray TCPConn::read()
{
	const QByteArray retval = data_->readData;
	data_->readData.resize(0);
	return retval;
}

void TCPConn::write(const QByteArray & data, bool notifyFinished)
{
	if (data_->tlsStream) data_->impl.tlsWrite(data_, data, notifyFinished);
	else                  data_->impl.writeToSocket(data_, data, notifyFinished);
}

void TCPConn::close(CloseType type, bool notifyClose)
{
	if (data_->tlsStream) data_->impl.tlsCloseConn(data_, type, notifyClose);
	else                  data_->impl.closeConn(data_, type, notifyClose);
}

void TCPConn::startReadWatcher()
{
	if (data_->tlsStream) data_->impl.tlsStartReadWatcher(data_);
	else                  data_->impl.startReadWatcher(data_);
}

TCPConn::CloseType TCPConn::isClosed() const
{
	return data_->closeType;
}

QByteArray TCPConn::peerIP() const
{
	return data_->peerIP;
}

quint16 TCPConn::peerPort() const
{
	return data_->peerPort;
}

TCPConnData * TCPConn::detach()
{
	TCPConnData * rv = data_;
	data_ = 0;
	rv->conn = 0;
	return rv;
}

void TCPConn::setNoDelay(bool noDelay)
{
	impl::TCPManagerImpl::setNoDelay(data_->socket, noDelay);
}

TCPManager & TCPConn::manager() const
{
	return data_->impl.parent;
}

}}	// namespace

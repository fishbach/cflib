/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
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

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

#include "tcpconn.h"

#include <cflib/crypt/tlsstream.h>
#include <cflib/net/impl/tcpconninitializer.h>
#include <cflib/net/impl/tcpmanagerimpl.h>
#include <cflib/net/tcpmanager.h>
#include <cflib/util/libev.h>

namespace cflib { namespace net {

TCPConn::TCPConn(const TCPConnInitializer * init, uint readBufferSize) :
	mgr_(init->mgr),
	socket_(init->socket), peerIP_(init->peerIP), peerPort_(init->peerPort),
	readWatcher_(new ev_io),
	writeWatcher_(new ev_io),
	readBuf_(readBufferSize, '\0'),
	closeAfterWriting_(false), notifyWrite_(false), closeType_(NotClosed),
	tlsStream_(init->tlsStream), tlsThreadId_(init->tlsThreadId)
{
	delete init;
	ev_io_init(readWatcher_, &TCPManager::Impl::readable, socket_, EV_READ);
	readWatcher_->data = this;
	ev_io_init(writeWatcher_, &TCPManager::Impl::writeable, socket_, EV_WRITE);
	writeWatcher_->data = this;

	if (tlsStream_) {
		const QByteArray data = tlsStream_->initialSend();
		if (!data.isEmpty()) mgr_.impl_->writeToSocket(this, data, false);
	}
}

TCPConn::~TCPConn()
{
	delete readWatcher_;
	delete writeWatcher_;
	delete tlsStream_;
}

void TCPConn::destroy()
{
	mgr_.impl_->destroy(this);
}

QByteArray TCPConn::read()
{
	const QByteArray retval = readData_;
	readData_.resize(0);
	return retval;
}

void TCPConn::startReadWatcher()
{
	mgr_.impl_->startReadWatcher(this);
}

void TCPConn::write(const QByteArray & data, bool notifyFinished)
{
	if (tlsStream_) mgr_.impl_->tlsWrite(this, data, notifyFinished);
	else            mgr_.impl_->writeToSocket(this, data, notifyFinished);
}

void TCPConn::close(CloseType type)
{
	if (tlsStream_) mgr_.impl_->tlsCloseConn(this, type);
	else            mgr_.impl_->closeConn(this, type);
}

const TCPConnInitializer * TCPConn::detachFromSocket()
{
	const TCPConnInitializer * rv = new TCPConnInitializer(mgr_, socket_, peerIP_, peerPort_, tlsStream_, tlsThreadId_);
	socket_ = -1;
	closeType_ = HardClosed;
	tlsStream_ = 0;
	return rv;
}

void TCPConn::setNoDelay(bool noDelay)
{
	TCPManager::Impl::setNoDelay(socket_, noDelay);
}

}}	// namespace

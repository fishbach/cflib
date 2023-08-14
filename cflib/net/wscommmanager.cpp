/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "wscommmanager.h"

namespace cflib { namespace net {

void WSCommManagerBase::send(uint connId, const QByteArray & data, bool isBinary)
{
	if (!verifyThreadCall(&WSCommManagerBase::send, connId, data, isBinary)) return;
	WebSocketService::send(connId, data, isBinary);
}

void WSCommManagerBase::close(uint connId, TCPConn::CloseType type)
{
	if (!verifyThreadCall(&WSCommManagerBase::close, connId, type)) return;
	WebSocketService::close(connId, type);
}

QByteArray WSCommManagerBase::getRemoteIP(uint connId)
{
	SyncedThreadCall<QByteArray> stc(this);
	if (!stc.verify(&WSCommManagerBase::getRemoteIP, connId)) return stc.retval();
	return WebSocketService::getRemoteIP(connId);
}

WSCommManagerBase::WSCommManagerBase(const QString & path, const QRegularExpression & allowedOrigin,
	uint connectionTimeoutSec)
:
	WebSocketService(path, allowedOrigin, connectionTimeoutSec)
{
}

}}	// namespace

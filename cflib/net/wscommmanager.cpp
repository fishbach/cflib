/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "wscommmanager.h"

namespace cflib { namespace net {

void WSCommManagerBase::saveHeaderField(const QByteArray & field)
{
    if (!verifyThreadCall(&WSCommManagerBase::saveHeaderField, field)) return;
    WebSocketService::saveHeaderField(field);
}

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

QByteArray WSCommManagerBase::getRemoteIP(uint connId) const
{
    SyncedThreadCall<QByteArray> stc(this);
    if (!stc.verify(&WSCommManagerBase::getRemoteIP, connId)) return stc.retval();
    return WebSocketService::getRemoteIP(connId);
}

QByteArray WSCommManagerBase::getHeader(uint connId, const QByteArray & header) const
{
    SyncedThreadCall<QByteArray> stc(this);
    if (!stc.verify(&WSCommManagerBase::getHeader, connId, header)) return stc.retval();
    return WebSocketService::getHeader(connId, header);
}

WSCommManagerBase::WSCommManagerBase(const QString & path, const QRegularExpression & allowedOrigin,
    uint connectionTimeoutSec)
:
    WebSocketService(path, allowedOrigin, connectionTimeoutSec)
{
}

}}    // namespace

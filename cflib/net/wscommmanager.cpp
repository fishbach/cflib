/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "wscommmanager.h"

namespace cflib { namespace net {

void WSCommManagerBase::saveHeaderField(const CFByteArray & field)
{
    if (!verifyThreadCall(&WSCommManagerBase::saveHeaderField, field)) return;
    WebSocketService::saveHeaderField(field);
}

void WSCommManagerBase::send(uint connId, const CFByteArray & data, bool isBinary)
{
    if (!verifyThreadCall(&WSCommManagerBase::send, connId, data, isBinary)) return;
    WebSocketService::send(connId, data, isBinary);
}

void WSCommManagerBase::close(uint connId, TCPConn::CloseType type)
{
    if (!verifyThreadCall(&WSCommManagerBase::close, connId, type)) return;
    WebSocketService::close(connId, type);
}

CFByteArray WSCommManagerBase::getRemoteIP(uint connId) const
{
    SyncedThreadCall<CFByteArray> stc(this);
    if (!stc.verify(&WSCommManagerBase::getRemoteIP, connId)) return stc.retval();
    return WebSocketService::getRemoteIP(connId);
}

CFByteArray WSCommManagerBase::getHeader(uint connId, const CFByteArray & header) const
{
    SyncedThreadCall<CFByteArray> stc(this);
    if (!stc.verify(&WSCommManagerBase::getHeader, connId, header)) return stc.retval();
    return WebSocketService::getHeader(connId, header);
}

WSCommManagerBase::WSCommManagerBase(const String & path, const CFRegex & allowedOrigin,
    uint connectionTimeoutSec)
:
    WebSocketService(path, allowedOrigin, connectionTimeoutSec)
{
}

}}    // namespace

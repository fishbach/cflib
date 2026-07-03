/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "wscommmanager.h"

USE_LOG(LogCat::Http)

namespace cflib::net {

// ----------------------------------------------------------------------------

void WSCommStateListener<void>::newConnection(uint connDataId, uint connId)
{
    CF_UNUSED(connDataId); CF_UNUSED(connId);
}

void WSCommStateListener<void>::connectionClosed(uint connDataId, uint connId, bool isLast)
{
    CF_UNUSED(connDataId); CF_UNUSED(connId); CF_UNUSED(isLast);
}

// ----------------------------------------------------------------------------

WSCommManagerBase::WSCommManagerBase(const String & path, const Regex & allowedOrigin,
    uint connectionTimeoutSec, uint sessionTimeoutSec)
:
    WebSocketService(path, allowedOrigin, connectionTimeoutSec),
    timer_(this, &WSCommManagerBase::checkTimeout),
    sessionTimeoutSec_(sessionTimeoutSec)
{
    init();
}

WSCommManagerBase::~WSCommManagerBase()
{
    stopVerifyThread();
}

void WSCommManagerBase::connDataOk(uint connDataId)
{
    if (!verifyThreadCall(&WSCommManagerBase::connDataOk, connDataId)) return;

    verifyConnData(connDataId);
}

void WSCommManagerBase::saveHeaderField(const ByteArray & field)
{
    if (!verifyThreadCall(&WSCommManagerBase::saveHeaderField, field)) return;
    WebSocketService::saveHeaderField(field);
}

void WSCommManagerBase::send(uint connId, const ByteArray & data, bool isBinary)
{
    if (!verifyThreadCall(&WSCommManagerBase::send, connId, data, isBinary)) return;
    WebSocketService::send(connId, data, isBinary);
}

void WSCommManagerBase::close(uint connId, TCPConn::CloseType type)
{
    if (!verifyThreadCall(&WSCommManagerBase::close, connId, type)) return;
    WebSocketService::close(connId, type);
}

ByteArray WSCommManagerBase::getRemoteIP(uint connId) const
{
    SyncedThreadCall<ByteArray> stc(this);
    if (!stc.verify(&WSCommManagerBase::getRemoteIP, connId)) return stc.retval();
    return WebSocketService::getRemoteIP(connId);
}

ByteArray WSCommManagerBase::getHeader(uint connId, const ByteArray & header) const
{
    SyncedThreadCall<ByteArray> stc(this);
    if (!stc.verify(&WSCommManagerBase::getHeader, connId, header)) return stc.retval();
    return WebSocketService::getHeader(connId, header);
}

List<uint> WSCommManagerBase::getAllConnIds() const
{
    SyncedThreadCall<List<uint>> stc(this);
    if (!stc.verify(&WSCommManagerBase::getAllConnIds)) return stc.retval();
    return connId2dataId_.keys();
}

uint WSCommManagerBase::getConnDataId(const ByteArray & clientId) const
{
    SyncedThreadCall<uint> stc(this);
    if (!stc.verify<WSCommManagerBase, const ByteArray &>(&WSCommManagerBase::getConnDataId, clientId)) return stc.retval();
    return clientIds_.value(clientId, 0u);
}

uint WSCommManagerBase::getConnDataId(uint connId) const
{
    SyncedThreadCall<uint> stc(this);
    if (!stc.verify<WSCommManagerBase, uint>(&WSCommManagerBase::getConnDataId, connId)) return stc.retval();
    return connId2dataId_.value(connId, 0u);
}

// ----------------------------------------------------------------------------

bool WSCommManager<void>::hasConnDataChecker() const
{
    return connDataChecker_ != 0;
}

void WSCommManager<void>::dispatchCheckConnData(uint connDataId, uint connId)
{
    if (connDataChecker_) connDataChecker_->checkConnData(connDataId, connId);
}

void WSCommManager<void>::dispatchNewConnection(uint connDataId, uint connId)
{
    for (auto * listener : stateListener_) listener->newConnection(connDataId, connId);
}

void WSCommManager<void>::dispatchConnectionClosed(uint connDataId, uint connId, bool isLast)
{
    for (auto * listener : stateListener_) listener->connectionClosed(connDataId, connId, isLast);
}

bool WSCommManager<void>::dispatchTextMsg(const ByteArray & data, uint connDataId, uint connId)
{
    for (auto * hdl : textMsgHandler_) {
        if (hdl->handleTextMsg(data, connDataId, connId)) return true;
    }
    return false;
}

bool WSCommManager<void>::dispatchMsg(uint64 tag,
    const ByteArray & data, int tagLen, int lengthSize, int32 valueLen,
    uint connDataId, uint connId)
{
    MsgHandler * hdl = msgHandler_.value(tag, (MsgHandler *)nullptr);
    if (!hdl) return false;
    hdl->handleMsg(tag, data, tagLen, lengthSize, valueLen, connDataId, connId);
    return true;
}

// ----------------------------------------------------------------------------

bool WSCommManagerBase::containsConnDataId(uint connDataId) const
{
    return connInfos_.contains(connDataId);
}

WSCommManagerBase::ConnIds WSCommManagerBase::connIdsOf(uint connDataId) const
{
    return connInfos_.value(connDataId).connIds;
}

bool WSCommManagerBase::verifyConnData(uint connDataId)
{
    ConnInfo & info = connInfos_[connDataId];
    if (info.connDataVerified) return true;
    info.connDataVerified = true;
    for (uint connId : info.connIds) {
        dispatchNewConnection(connDataId, connId);
        continueRead(connId);
    }
    return false;
}

// ----------------------------------------------------------------------------

void WSCommManagerBase::newMsg(uint connId, const ByteArray & data, bool isBinary, bool & stopRead)
{
    const uint dataId = connId2dataId_.value(connId, 0u);

    // handle text msg
    if (!isBinary) {
        if (dataId == 0) {
            close(connId, TCPConn::HardClosed);
            logInfo("request without clientId from %1", connId);
            return;
        }

        if (!dispatchTextMsg(data, dataId, connId)) {
            close(connId, TCPConn::HardClosed);
            logInfo("unhandled text message from %1", connId);
        }
        return;
    }

    // read outer BER
    uint64 tag = 0;
    int tagLen = 0;
    int lengthSize = 0;
    const int32 valueLen = serialize::getTLVLength(data, tag, tagLen, lengthSize);
    if (valueLen < 0) {
        close(connId, TCPConn::HardClosed);
        logInfo("broken BER msg %1 (%2)", connId, valueLen);
        return;
    }
    logTrace("ws msg (connId: %1, tag: %2, valueLen: %3)", connId, tag, valueLen);

    // handle new connections
    if (dataId == 0) {
        if (tag == 1) {
            uint dId;
            if (valueLen != 20) {
                dId = sendNewClientId(connId, stopRead);
            } else {
                const ByteArray clId = serialize::fromByteArray<ByteArray>(data, tagLen, lengthSize, valueLen);
                dId = clientIds_.value(clId, 0u);
                if (dId == 0) {
                    dId = sendNewClientId(connId, stopRead);
                } else {
                    connId2dataId_[connId] = dId;
                    ConnInfo & info = connInfos_[dId];
                    info.connIds << connId;
                    if (!info.connDataVerified) {
                        if (!hasConnDataChecker()) {
                            info.connDataVerified = true;
                        } else {
                            stopRead = true;
                            execLater([this, dId, connId]() { dispatchCheckConnData(dId, connId); });
                        }
                    }
                }
            }
            if (stopRead) return;

            // inform state listener
            dispatchNewConnection(dId, connId);
        } else {
            close(connId, TCPConn::HardClosed);
            logInfo("request without clientId from %1", connId);
        }
        return;
    }

    // ping
    if (tag == 4) {
        send(connId, data, true);
        return;
    }

    // handler
    if (!dispatchMsg(tag, data, tagLen, lengthSize, valueLen, dataId, connId)) {
        logInfo("unhandled message from %1 (tag: %2)", connId, tag);
    }
}

void WSCommManagerBase::closed(uint connId, TCPConn::CloseType)
{
    // no partial close on websockets
    close(connId, TCPConn::ReadWriteClosed);

    // Do we know anything?
    const uint dataId = connId2dataId_.value(connId, 0u);
    if (dataId == 0) return;

    connId2dataId_.erase(connId);
    ConnInfo & info = connInfos_[dataId];
    info.connIds.erase(connId);
    const bool isLast = info.connIds.isEmpty();
    if (isLast) {
        info.connDataVerified = false;
        info.lastClosed = DateTime::currentDateTimeUtc();
    }

    // inform state listener
    dispatchConnectionClosed(dataId, connId, isLast);
}

void WSCommManagerBase::init()
{
    if (!verifyThreadCall(&WSCommManagerBase::init)) return;
    timer_.start(sessionTimeoutSec_ / 10.0);
}

uint WSCommManagerBase::sendNewClientId(uint connId, bool & stopRead)
{
    // create clientId
    const ByteArray clId = crypt::random(20);

    // get free id
    uint dataId;
    do {
        dataId = crypt::randomUInt32();
    } while (dataId == 0 || connInfos_.contains(dataId));
    ConnInfo & info = connInfos_[dataId];

    connId2dataId_[connId] = dataId;
    info.connIds << connId;
    clientIds_[clId] = dataId;
    if (hasConnDataChecker()) {
        info.connDataVerified = false;
        stopRead = true;
        execLater([this, dataId, connId]() { dispatchCheckConnData(dataId, connId); });
    }
    send(connId, serialize::toByteArray(clId, 1), true);
    return dataId;
}

void WSCommManagerBase::checkTimeout()
{
    logFunctionTrace

    Set<uint> removedIds;
    {
        const DateTime now = DateTime::currentDateTimeUtc();
        for (auto it = connInfos_.begin(); it != connInfos_.end(); ) {
            ConnInfo & info = it->second;
            if (info.connIds.isEmpty() && info.lastClosed.secsTo(now) > (int64)sessionTimeoutSec_) {
                removedIds.insert(it->first);
                it = connInfos_.erase(it);
            } else ++it;
        }
    }

    if (!removedIds.isEmpty()) {
        for (auto it = clientIds_.begin(); it != clientIds_.end(); ) {
            if (removedIds.contains(it->second)) it = clientIds_.erase(it);
            else ++it;
        }
        for (uint connDataId : removedIds) connDataIdRemoved(connDataId);
        logDebug("timeout of %1 sessions", removedIds.size());
    }
}

} // namespace

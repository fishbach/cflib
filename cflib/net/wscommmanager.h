/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/crypt/util.h>
#include <cflib/net/websocketservice.h>
#include <cflib/serialize/util.h>
#include <cflib/util/evtimer.h>
#include <cflib/util/log.h>

namespace cflib::net {

template<typename C> class WSCommManager;

template<typename C>
class WSCommConnMgrAccess
{
public:
    WSCommConnMgrAccess() : mgr_(0) {}

protected:
    inline WSCommManager<C> & commMgr() { return *mgr_; }

private:
    WSCommManager<C> * mgr_;
    friend class WSCommManager<C>;
};

template<typename C>
class WSCommConnDataChecker : public virtual WSCommConnMgrAccess<C>
{
public:
    virtual void checkConnData(const C & connData, uint connDataId, uint connId) = 0;
};

template<typename C>
class WSCommStateListener : public virtual WSCommConnMgrAccess<C>
{
public:
    virtual void newConnection(const C & connData, uint connDataId, uint connId);
    virtual void connDataChange(const C & oldConnData, const C & newConnData, uint connDataId, const Set<uint> & connIds);
    virtual void connectionClosed(const C & connData, uint connDataId, uint connId, bool isLast);
};

// ----------------------------------------------------------------------------

template<typename C>
class WSCommTextMsgHandler : public virtual WSCommConnMgrAccess<C>
{
public:
    virtual bool handleTextMsg(const ByteArray & data, const C & connData, uint connId) = 0;
};

template<typename C>
class WSCommMsgHandler : public virtual WSCommConnMgrAccess<C>
{
public:
    virtual void handleMsg(uint64 tag,
        const ByteArray & data, int tagLen, int lengthSize, int32 valueLen,
        const C & connData, uint connDataId, uint connId) = 0;
};

// ----------------------------------------------------------------------------

class WSCommManagerBase : public WebSocketService
{
public:
    void saveHeaderField(const ByteArray & field);

    void send(uint connId, const ByteArray & data, bool isBinary);
    void close(uint connId, TCPConn::CloseType type = TCPConn::ReadWriteClosed);

    ByteArray getRemoteIP(uint connId) const;
    ByteArray getHeader(uint connId, const ByteArray & header) const;

protected:
    WSCommManagerBase(const String & path, const Regex & allowedOrigin, uint connectionTimeoutSec);
};

/*
 * Tags:
 * 1 : client id (s <-> c)
 * 2 : rmi       (s <-> c)
 * 3 : rsig      (s  -> c)
 * 4 : ping      (s <-> c)
 */

template<typename C>
class WSCommManager : public WSCommManagerBase
{
    USE_LOG_MEMBER(LogCat::Http)
public:
    typedef WSCommConnDataChecker<C> ConnDataChecker;
    typedef WSCommStateListener  <C> StateListener;
    typedef WSCommTextMsgHandler <C> TextMsgHandler;
    typedef WSCommMsgHandler     <C> MsgHandler;
    typedef Set<uint> ConnIds;

public:
    WSCommManager(const String & path, const Regex & allowedOrigin = Regex(),
        uint connectionTimeoutSec = 10, uint sessionTimeoutSec = 86400);
    ~WSCommManager();

    void setConnDataChecker(ConnDataChecker & checker)     { connDataChecker_ = &checker; checker.mgr_ = this; }
    void registerStateListener (StateListener & listener)  { stateListener_ << &listener; listener.mgr_ = this; }
    void registerTextMsgHandler(TextMsgHandler & hdl)      { textMsgHandler_ << &hdl; hdl.mgr_ = this; }
    void registerMsgHandler(uint64 tag, MsgHandler & hdl) { msgHandler_[tag] = &hdl; hdl.mgr_ = this; }
    void updateConnData(uint connDataId, const C & connData);
    void connDataOk(uint connDataId);
    void getConnData(const ByteArray & clientId, C & connData, uint & connDataId);
    void getConnData(uint connId, C & connData, uint & connDataId);

protected:
    virtual void newMsg(uint connId, const ByteArray & data, bool isBinary, bool & stopRead);
    virtual void closed(uint connId, TCPConn::CloseType type);

private:
    class ConnInfo {
    public:
        ConnInfo() : connData(), connDataVerified(true) {}
        C connData;
        ConnIds connIds;
        DateTime lastClosed;
        bool connDataVerified;
    };

private:
    void init();
    uint sendNewClientId(uint connId, bool & stopRead);
    void checkTimeout();
    bool connDataOk(ConnInfo & info, uint connDataId);

private:
    ConnDataChecker * connDataChecker_;
    List<StateListener *> stateListener_;
    List<TextMsgHandler *> textMsgHandler_;
    Hash<uint64, MsgHandler *> msgHandler_;

    Hash<uint, uint> connId2dataId_;
    Hash<uint, ConnInfo> connInfos_;
    Map<ByteArray, uint> clientIds_;

    util::EVTimer timer_;
    uint sessionTimeoutSec_;
};

// ============================================================================

template<typename C>
void WSCommStateListener<C>::newConnection(const C & connData, uint connDataId, uint connId)
{
    CF_UNUSED(connData); CF_UNUSED(connDataId); CF_UNUSED(connId);
}

template<typename C>
void WSCommStateListener<C>::connDataChange(const C & oldConnData, const C & newConnData,
    uint connDataId, const Set<uint> & connIds)
{
    CF_UNUSED(oldConnData); CF_UNUSED(newConnData); CF_UNUSED(connDataId); CF_UNUSED(connIds);
}

template<typename C>
void WSCommStateListener<C>::connectionClosed(const C & connData, uint connDataId, uint connId, bool isLast)
{
    CF_UNUSED(connData); CF_UNUSED(connDataId); CF_UNUSED(connId); CF_UNUSED(isLast);
}

// ----------------------------------------------------------------------------

template<typename C>
WSCommManager<C>::WSCommManager(const String & path, const Regex & allowedOrigin,
    uint connectionTimeoutSec, uint sessionTimeoutSec)
:
    WSCommManagerBase(path, allowedOrigin, connectionTimeoutSec),
    connDataChecker_(0),
    timer_(this, &WSCommManager::checkTimeout), sessionTimeoutSec_(sessionTimeoutSec)
{
    init();
}

template<typename C>
WSCommManager<C>::~WSCommManager()
{
    stopVerifyThread();
}

template<typename C>
void WSCommManager<C>::updateConnData(uint connDataId, const C & connData)
{
    if (!verifyThreadCall(&WSCommManager<C>::updateConnData, connDataId, connData)) return;

    if (!connInfos_.contains(connDataId)) return;
    ConnInfo & info = connInfos_[connDataId];
    C oldConnData = info.connData;
    info.connData = connData;
    if (connDataOk(info, connDataId)) {
        // inform state listener
        for (auto * listener : stateListener_) listener->connDataChange(oldConnData, connData, connDataId, info.connIds);
    }
}

template<typename C>
void WSCommManager<C>::connDataOk(uint connDataId)
{
    if (!verifyThreadCall(&WSCommManager<C>::connDataOk, connDataId)) return;

    connDataOk(connInfos_[connDataId], connDataId);
}

template<typename C>
void WSCommManager<C>::getConnData(const ByteArray & clientId, C & connData, uint & connDataId)
{
    if (!verifySyncedThreadCall<WSCommManager<C>, const ByteArray &>(&WSCommManager<C>::getConnData, clientId, connData, connDataId)) return;

    connDataId = mapValue(clientIds_, clientId, 0u);
    connData = connDataId == 0 ? C() : hashValue(connInfos_, connDataId).connData;
}

template<typename C>
void WSCommManager<C>::getConnData(uint connId, C & connData, uint & connDataId)
{
    if (!verifySyncedThreadCall<WSCommManager<C>, uint>(&WSCommManager<C>::getConnData, connId, connData, connDataId)) return;

    connDataId = hashValue(connId2dataId_, connId, 0u);
    connData = connDataId == 0 ? C() : hashValue(connInfos_, connDataId).connData;
}

template<typename C>
bool WSCommManager<C>::connDataOk(WSCommManager::ConnInfo & info, uint connDataId)
{
    if (info.connDataVerified) return true;
    info.connDataVerified = true;
    for (uint connId : info.connIds) {
        for (auto * listener : stateListener_) listener->newConnection(info.connData, connDataId, connId);
        continueRead(connId);
    }
    return false;
}

template<typename C>
void WSCommManager<C>::newMsg(uint connId, const ByteArray & data, bool isBinary, bool & stopRead)
{
    const uint dataId = hashValue(connId2dataId_, connId, 0u);

    // handle text msg
    if (!isBinary) {
        if (dataId == 0) {
            close(connId, TCPConn::HardClosed);
            logInfo("request without clientId from %1", connId);
            return;
        }

        for (auto * hdl : textMsgHandler_) {
            if (hdl->handleTextMsg(data, connInfos_[dataId].connData, connId)) return;
        }
        close(connId, TCPConn::HardClosed);
        logInfo("unhandled text message from %1", connId);
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
                dId = mapValue(clientIds_, clId, 0u);
                if (dId == 0) {
                    dId = sendNewClientId(connId, stopRead);
                } else {
                    connId2dataId_[connId] = dId;
                    ConnInfo & info = connInfos_[dId];
                    info.connIds << connId;
                    if (!info.connDataVerified) {
                        if (!connDataChecker_) {
                            info.connDataVerified = true;
                        } else {
                            stopRead = true;
                            execLater([connDataChecker = connDataChecker_, connData = info.connData, dId, connId]() {
                                connDataChecker->checkConnData(connData, dId, connId);
                            });
                        }
                    }
                }
            }
            if (stopRead) return;

            // inform state listener
            for (auto * listener : stateListener_) listener->newConnection(connInfos_[dId].connData, dId, connId);
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
    MsgHandler * hdl = hashValue(msgHandler_, tag, (MsgHandler *)nullptr);
    if (hdl) {
        hdl->handleMsg(tag, data, tagLen, lengthSize, valueLen, connInfos_[dataId].connData, dataId, connId);
        return;
    }

    logInfo("unhandled message from %1 (tag: %2)", connId, tag);
}

template<typename C>
void WSCommManager<C>::closed(uint connId, TCPConn::CloseType)
{
    // no partial close on websockets
    close(connId, TCPConn::ReadWriteClosed);

    // Do we know anything?
    const uint dataId = hashValue(connId2dataId_, connId, 0u);
    if (dataId == 0) return;

    connId2dataId_.erase(connId);
    ConnInfo & info = connInfos_[dataId];
    info.connIds.erase(connId);
    const bool isLast = info.connIds.empty();
    if (isLast) {
        info.connDataVerified = false;
        info.lastClosed = DateTime::currentDateTimeUtc();
    }

    // inform state listener
    for (auto * listener : stateListener_) listener->connectionClosed(info.connData, dataId, connId, isLast);
}

template<typename C>
void WSCommManager<C>::init()
{
    if (!verifyThreadCall(&WSCommManager::init)) return;
    timer_.start(sessionTimeoutSec_ / 10.0);
}

template<typename C>
uint WSCommManager<C>::sendNewClientId(uint connId, bool & stopRead)
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
    if (connDataChecker_) {
        info.connDataVerified = false;
        stopRead = true;
        execLater([connDataChecker = connDataChecker_, connData = info.connData, dataId, connId]() {
            connDataChecker->checkConnData(connData, dataId, connId);
        });
    }
    send(connId, serialize::toByteArray(clId, 1), true);
    return dataId;
}

template<typename C>
void WSCommManager<C>::checkTimeout()
{
    logFunctionTrace

    Set<uint> removedIds;
    {
        const DateTime now = DateTime::currentDateTimeUtc();
        for (auto it = connInfos_.begin(); it != connInfos_.end(); ) {
            ConnInfo & info = it->second;
            if (info.connIds.empty() && info.lastClosed.secsTo(now) > (int64)sessionTimeoutSec_) {
                removedIds.insert(it->first);
                it = connInfos_.erase(it);
            } else ++it;
        }
    }

    if (!removedIds.empty()) {
        for (auto it = clientIds_.begin(); it != clientIds_.end(); ) {
            if (contains(removedIds, it->second)) it = clientIds_.erase(it);
            else ++it;
        }
        logDebug("timeout of %1 sessions", removedIds.size());
    }
}

} // namespace

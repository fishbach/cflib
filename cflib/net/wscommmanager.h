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

// ----------------------------------------------------------------------------

class WSCommManagerBase;
template<typename C> class WSCommManager;

template<typename C>
class WSCommConnMgrAccess
{
public:
    WSCommConnMgrAccess() : mgr_(nullptr) {}

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

template<>
class WSCommConnDataChecker<void> : public virtual WSCommConnMgrAccess<void>
{
public:
    virtual void checkConnData(uint connDataId, uint connId) = 0;
};

template<typename C>
class WSCommStateListener : public virtual WSCommConnMgrAccess<C>
{
public:
    virtual void newConnection   (const C & connData, uint connDataId, uint connId);
    virtual void connDataChange  (const C & oldConnData, const C & newConnData, uint connDataId, const Set<uint> & connIds);
    virtual void connectionClosed(const C & connData, uint connDataId, uint connId, bool isLast);
};

template<>
class WSCommStateListener<void> : public virtual WSCommConnMgrAccess<void>
{
public:
    virtual void newConnection   (uint connDataId, uint connId);
    virtual void connectionClosed(uint connDataId, uint connId, bool isLast);
};

// ----------------------------------------------------------------------------

template<typename C>
class WSCommTextMsgHandler : public virtual WSCommConnMgrAccess<C>
{
public:
    virtual bool handleTextMsg(const ByteArray & data, const C & connData, uint connDataId, uint connId) = 0;
};

template<>
class WSCommTextMsgHandler<void> : public virtual WSCommConnMgrAccess<void>
{
public:
    virtual bool handleTextMsg(const ByteArray & data, uint connDataId, uint connId) = 0;
};

template<typename C>
class WSCommMsgHandler : public virtual WSCommConnMgrAccess<C>
{
public:
    virtual void handleMsg(uint64 tag,
        const ByteArray & data, int tagLen, int lengthSize, int32 valueLen,
        const C & connData, uint connDataId, uint connId) = 0;
};

template<>
class WSCommMsgHandler<void> : public virtual WSCommConnMgrAccess<void>
{
public:
    virtual void handleMsg(uint64 tag,
        const ByteArray & data, int tagLen, int lengthSize, int32 valueLen,
        uint connDataId, uint connId) = 0;
};

// ============================================================================

// Tags:
// 1 : client id (s <-> c)
// 2 : rmi       (s <-> c)
// 3 : rsig      (s  -> c)
// 4 : ping      (s <-> c)
// 5 : alive     (s  -> c)

class WSCommManagerBase : public WebSocketService
{
public:
    typedef WSCommConnDataChecker<void> ConnDataChecker;
    typedef WSCommStateListener<void>   StateListener;
    typedef WSCommTextMsgHandler<void>  TextMsgHandler;
    typedef WSCommMsgHandler<void>      MsgHandler;
    typedef Set<uint> ConnIds;

public:
    ~WSCommManagerBase();

    void connDataOk(uint connDataId);

    void saveHeaderField(const ByteArray & field);

    void send(uint connId, const ByteArray & data, bool isBinary);
    void close(uint connId, TCPConn::CloseType type = TCPConn::ReadWriteClosed);

    ByteArray getRemoteIP(uint connId) const;
    ByteArray getHeader(uint connId, const ByteArray & header) const;

    List<uint> getAllConnIds() const;
    uint getConnDataId(const ByteArray & clientId) const;
    uint getConnDataId(uint connId) const;

protected:
    WSCommManagerBase(const String & path, const Regex & allowedOrigin, uint connectionTimeoutSec, uint sessionTimeoutSec);

    void newMsg(uint connId, const ByteArray & data, bool isBinary, bool & stopRead) override;
    void closed(uint connId, TCPConn::CloseType type) override;

    virtual bool hasConnDataChecker() const = 0;
    virtual void dispatchCheckConnData(uint connDataId, uint connId) = 0;
    virtual void dispatchNewConnection(uint connDataId, uint connId) = 0;
    virtual void dispatchConnectionClosed(uint connDataId, uint connId, bool isLast) = 0;
    virtual bool dispatchTextMsg(const ByteArray & data, uint connDataId, uint connId) = 0;
    virtual bool dispatchMsg(uint64 tag,
        const ByteArray & data, int tagLen, int lengthSize, int32 valueLen,
        uint connDataId, uint connId) = 0;
    virtual void connDataIdRemoved(uint connDataId) = 0;

    bool containsConnDataId(uint connDataId) const;
    ConnIds connIdsOf(uint connDataId) const;
    bool verifyConnData(uint connDataId);

protected:
    class ConnInfo {
    public:
        ConnInfo() : connDataVerified(true) {}
        ConnIds connIds;
        DateTime lastClosed;
        bool connDataVerified;
    };

    Hash<uint, uint> connId2dataId_;
    Map<ByteArray, uint> clientIds_;

private:
    void init();
    uint sendNewClientId(uint connId, bool & stopRead);
    void checkTimeout();

private:
    Hash<uint, ConnInfo> connInfos_;

    util::EVTimer timer_;
    uint sessionTimeoutSec_;
};

// ============================================================================

template<typename C>
class WSCommManager : public WSCommManagerBase
{
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

    void setConnDataChecker(ConnDataChecker & checker)    { connDataChecker_ = &checker; checker.mgr_ = this; }
    void registerStateListener (StateListener & listener) { stateListener_ << &listener; listener.mgr_ = this; }
    void registerTextMsgHandler(TextMsgHandler & hdl)     { textMsgHandler_ << &hdl; hdl.mgr_ = this; }
    void registerMsgHandler(uint64 tag, MsgHandler & hdl) { msgHandler_[tag] = &hdl; hdl.mgr_ = this; }
    void updateConnData(uint connDataId, const C & connData);
    void getConnData(const ByteArray & clientId, C & connData, uint & connDataId);
    void getConnData(uint connId, C & connData, uint & connDataId);

protected:
    bool hasConnDataChecker() const override { return connDataChecker_ != 0; }

    void dispatchCheckConnData(uint connDataId, uint connId) override
    {
        if (connDataChecker_) connDataChecker_->checkConnData(connData_[connDataId], connDataId, connId);
    }

    void dispatchNewConnection(uint connDataId, uint connId) override
    {
        const C & connData = connData_[connDataId];
        for (auto * listener : stateListener_) listener->newConnection(connData, connDataId, connId);
    }

    void dispatchConnectionClosed(uint connDataId, uint connId, bool isLast) override
    {
        const C & connData = connData_[connDataId];
        for (auto * listener : stateListener_) listener->connectionClosed(connData, connDataId, connId, isLast);
    }

    bool dispatchTextMsg(const ByteArray & data, uint connDataId, uint connId) override
    {
        const C & connData = connData_[connDataId];
        for (auto * hdl : textMsgHandler_) {
            if (hdl->handleTextMsg(data, connData, connDataId, connId)) return true;
        }
        return false;
    }

    bool dispatchMsg(uint64 tag,
        const ByteArray & data, int tagLen, int lengthSize, int32 valueLen,
        uint connDataId, uint connId) override
    {
        MsgHandler * hdl = msgHandler_.value(tag, (MsgHandler *)nullptr);
        if (!hdl) return false;
        hdl->handleMsg(tag, data, tagLen, lengthSize, valueLen, connData_[connDataId], connDataId, connId);
        return true;
    }

    void connDataIdRemoved(uint connDataId) override { connData_.erase(connDataId); }

private:
    ConnDataChecker *          connDataChecker_ = nullptr;
    List<StateListener *>      stateListener_;
    List<TextMsgHandler *>     textMsgHandler_;
    Hash<uint64, MsgHandler *> msgHandler_;

    Hash<uint, C> connData_;
};

template<>
class WSCommManager<void> : public WSCommManagerBase
{
public:
    using WSCommManagerBase::WSCommManagerBase;

    void setConnDataChecker(ConnDataChecker & checker)    { connDataChecker_ = &checker;  checker.mgr_  = this; }
    void registerStateListener (StateListener & listener) { stateListener_  << &listener; listener.mgr_ = this; }
    void registerTextMsgHandler(TextMsgHandler & hdl)     { textMsgHandler_ << &hdl;      hdl.mgr_      = this; }
    void registerMsgHandler(uint64 tag, MsgHandler & hdl) { msgHandler_[tag] = &hdl;      hdl.mgr_      = this; }

protected:
    bool hasConnDataChecker() const override;
    void dispatchCheckConnData(uint connDataId, uint connId) override;
    void dispatchNewConnection(uint connDataId, uint connId) override;
    void dispatchConnectionClosed(uint connDataId, uint connId, bool isLast) override;
    bool dispatchTextMsg(const ByteArray & data, uint connDataId, uint connId) override;
    bool dispatchMsg(uint64 tag,
        const ByteArray & data, int tagLen, int lengthSize, int32 valueLen,
        uint connDataId, uint connId) override;

private:
    ConnDataChecker *          connDataChecker_ = nullptr;
    List<StateListener *>      stateListener_;
    List<TextMsgHandler *>     textMsgHandler_;
    Hash<uint64, MsgHandler *> msgHandler_;
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
    WSCommManagerBase(path, allowedOrigin, connectionTimeoutSec, sessionTimeoutSec)
{
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

    if (!containsConnDataId(connDataId)) return;
    C oldConnData = connData_.value(connDataId);
    connData_[connDataId] = connData;
    if (verifyConnData(connDataId)) {
        // inform state listener
        const ConnIds ids = connIdsOf(connDataId);
        for (auto * listener : stateListener_) listener->connDataChange(oldConnData, connData, connDataId, ids);
    }
}

template<typename C>
void WSCommManager<C>::getConnData(const ByteArray & clientId, C & connData, uint & connDataId)
{
    if (!verifySyncedThreadCall<WSCommManager<C>, const ByteArray &>(&WSCommManager<C>::getConnData, clientId, connData, connDataId)) return;

    connDataId = clientIds_.value(clientId, 0u);
    connData = connDataId == 0 ? C() : connData_.value(connDataId);
}

template<typename C>
void WSCommManager<C>::getConnData(uint connId, C & connData, uint & connDataId)
{
    if (!verifySyncedThreadCall<WSCommManager<C>, uint>(&WSCommManager<C>::getConnData, connId, connData, connDataId)) return;

    connDataId = connId2dataId_.value(connId, 0u);
    connData = connDataId == 0 ? C() : connData_.value(connDataId);
}

} // namespace

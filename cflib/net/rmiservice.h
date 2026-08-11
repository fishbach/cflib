/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/net/rsig.h>
#include <cflib/serialize/serialize.h>
#include <cflib/serialize/serializeber.h>
#include <cflib/util/threadverify.h>

namespace cflib::net {

namespace impl { class RMIServerBase; }

class RMIReplier
{
public:
    RMIReplier(const RMIReplier &  other) = default;
    RMIReplier(      RMIReplier && other) = default;
    RMIReplier & operator=(const RMIReplier &  other) = delete;
    RMIReplier & operator=(      RMIReplier && other) = delete;

    void send();
    uint connId() const { return connId_; }

    template<typename T>
    inline RMIReplier & operator<<(const T & cl             ) { ser_ << cl; return *this; }
    inline RMIReplier & operator<<(serialize::Placeholder ph) { ser_ << ph; return *this; }

private:
    RMIReplier(impl::RMIServerBase & server, uint connId) : server_(server), connId_(connId), ser_(2) {}
    impl::RMIServerBase & server_;
    const uint connId_;
    serialize::BERSerializer ser_;
    friend class RMIServiceBase;
};

template<typename C> class RMIService;

class RMIServiceBase : public util::ThreadVerify
{
public:
    template<typename F, typename R>
    using rsig = cflib::net::RSig<F, R>;

public:
    virtual cflib::serialize::SerializeTypeInfo getServiceInfo() const = 0;

protected:
    inline uint connDataId() const { return connDataId_; }
    inline uint connId()     const { return connId_;     }
    RMIReplier delayReply();
    ByteArray getRemoteIP() const;
    virtual void preCallInit() {}
    virtual void connectionClosed(bool isLast) { CF_UNUSED(isLast); }

protected:
    virtual void processRMIServiceCallImpl(serialize::BERDeserializer & deser, uint callNo) = 0;
    virtual void processRMIServiceCallImpl(serialize::BERDeserializer & deser, uint callNo,
        serialize::BERSerializer & ser) = 0;
    virtual RSigBase * getCfSignal(uint sigNo) = 0;

private:
    RMIServiceBase(const String & threadName, uint threadCount, LoopType loopType);
    RMIServiceBase(ThreadVerify * other);

    void processRMIServiceCall(serialize::BERDeserializer deser, uint callNo, uint type, uint connDataId, uint connId);
    void connectionClosed(uint connDataId, uint connId, bool isLast);

private:
    impl::RMIServerBase * server_ = nullptr;
    static inline thread_local uint connDataId_ = 0;
    static inline thread_local uint connId_ = 0;
    static inline thread_local bool delayedReply_ = false;
    friend class impl::RMIServerBase;
    template<typename C> friend class RMIService;
};

template<typename C>
class RMIService : public RMIServiceBase
{
protected:
    RMIService(const String & threadName, uint threadCount = 1, LoopType loopType = Worker) :
        RMIServiceBase(threadName, threadCount, loopType) {}
    RMIService(ThreadVerify * other) : RMIServiceBase(other) {}

    using RMIServiceBase::connectionClosed;    // prevent hidden virtual warning

    inline const C & connData() const { return connData_; }
    virtual void connDataChange() {}

private:
    void processRMIServiceCall(serialize::BERDeserializer deser, uint callNo, uint type,
        const C & connData, uint connDataId, uint connId)
    {
        if (!verifyThreadCall(&RMIService::processRMIServiceCall, deser, callNo, type,
            connData, connDataId, connId)) return;

        connData_ = connData;
        RMIServiceBase::processRMIServiceCall(deser, callNo, type, connDataId, connId);
        connData_ = C{};
    }

    void connDataChange(const C & connData, uint connDataId, const Set<uint> & connIds)
    {
        if (!verifyThreadCall(&RMIService::connDataChange, connData, connDataId, connIds)) return;

        connData_   = connData;
        connDataId_ = connDataId;
        for (uint connId : connIds) {
            connId_ = connId;
            connDataChange();
        }
        connId_     = 0;
        connDataId_ = 0;
        connData_   = C{};
    }

    void connectionClosed(const C & connData, uint connDataId, uint connId, bool isLast)
    {
        if (!verifyThreadCall(&RMIService::connectionClosed, connData, connDataId, connId, isLast)) return;

        connData_ = connData;
        RMIServiceBase::connectionClosed(connDataId, connId, isLast);
        connData_ = C{};
    }

private:
    static inline thread_local C connData_;
    friend class impl::RMIServerBase;
};

template<>
class RMIService<void> : public RMIServiceBase
{
protected:
    RMIService(const String & threadName, uint threadCount = 1, LoopType loopType = Worker);
    RMIService(ThreadVerify * other);
};

} // namespace

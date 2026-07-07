/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "rmiservice.h"

#include <cflib/net/impl/rmiserverbase.h>

#include <algorithm>

namespace cflib::net {

RMIServiceBase::RMIServiceBase(const String & threadName, uint threadCount, LoopType loopType) :
    util::ThreadVerify(threadName, loopType, threadCount)
{
}

RMIServiceBase::RMIServiceBase(util::ThreadVerify * other) :
    util::ThreadVerify(other)
{
}

void RMIServiceBase::processRMIServiceCall(serialize::BERDeserializer deser, uint callNo, uint type, uint connDataId, uint connId)
{
    if (!verifyThreadCall(&RMIServiceBase::processRMIServiceCall, deser, callNo, type, connDataId, connId)) return;

    connDataId_ = connDataId;
    connId_     = connId;
    preCallInit();
    if (type == 0) {
        processRMIServiceCallImpl(deser, callNo);
        delayedReply_ = false;
    } else if (type == 1) {
        serialize::BERSerializer ser(2);
        processRMIServiceCallImpl(deser, callNo, ser);
        if (delayedReply_) delayedReply_ = false;
        else               server_->send(connId_, ser.data());
    } else {    // cfsignals
        RSigBase & sig = *getCfSignal(callNo);
        if (deser.get<bool>()) sig.  regClient(connDataId, connId, deser);
        else                   sig.unregClient(connDataId, connId, deser);
        return;
    }
    connId_     = 0;
    connDataId_ = 0;
}

void RMIServiceBase::connectionClosed(uint connDataId, uint connId, bool isLast)
{
    if (!verifyThreadCall(&RMIServiceBase::connectionClosed, connDataId, connId, isLast)) return;

    // remove rsig clients
    serialize::BERDeserializer deser{ByteArray{}};
    int i = getServiceInfo().cfSignals.size();
    while (i > 0) {
        cflib::net::RSigBase * sig = getCfSignal(i--);
        sig->unregClient(connDataId, connId, deser);
    }

    connDataId_ = connDataId;
    connId_     = connId;
    connectionClosed(isLast);
    connId_     = 0;
    connDataId_ = 0;
}

RMIReplier RMIServiceBase::delayReply()
{
    delayedReply_ = true;
    return RMIReplier(*server_, connId_);
}

ByteArray RMIServiceBase::getRemoteIP() const
{
    return server_->getRemoteIP(connId_);
}

void RMIReplier::send()
{
    server_.send(connId_, *this);
}

RMIService<void>::RMIService(const String & threadName, uint threadCount, LoopType loopType) :
    RMIServiceBase(threadName, threadCount, loopType)
{
}

RMIService<void>::RMIService(ThreadVerify * other) :
    RMIServiceBase(other)
{
}

} // namespace

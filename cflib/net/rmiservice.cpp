/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "rmiservice.h"

#include <cflib/net/impl/rmiserverbase.h>

#include <algorithm>

namespace cflib { namespace net {

RMIServiceBase::RMIServiceBase(const String & threadName, uint threadCount, LoopType loopType) :
    util::ThreadVerify(threadName, loopType, threadCount),
    server_(0), connId_(this, 0), delayedReply_(this, false)
{
}

RMIServiceBase::RMIServiceBase(util::ThreadVerify * other) :
    util::ThreadVerify(other),
    server_(0), connId_(this, 0), delayedReply_(this, false)
{
}

void RMIServiceBase::processRMIServiceCall(serialize::BERDeserializer deser, uint callNo, uint type, uint connId)
{
    if (!verifyThreadCall(&RMIServiceBase::processRMIServiceCall, deser, callNo, type, connId)) return;

    connId_ = connId;
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
        if (deser.get<bool>()) sig.  regClient(connId, deser);
        else                   sig.unregClient(connId, deser);
        return;
    }
    connId_ = 0;
}

void RMIServiceBase::connectionClosed(uint connId, bool isLast)
{
    if (!verifyThreadCall(&RMIServiceBase::connectionClosed, connId, isLast)) return;

    // remove rsig clients
    int i = getServiceInfo().cfSignals.size();
    while (i > 0) {
        auto & listeners = getCfSignal(i--)->defaultListeners;
        listeners.erase(
            std::remove_if(listeners.begin(), listeners.end(),
                [connId](const net::RSigBase::ConnIdRegId & p) { return p.first == connId; }),
            listeners.end());
    }

    connId_ = connId;
    connectionClosed(isLast);
    connId_ = 0;
}

RMIReplier RMIServiceBase::delayReply()
{
    delayedReply_ = true;
    return RMIReplier(*server_, connId_);
}

CFByteArray RMIServiceBase::getRemoteIP() const
{
    return server_->getRemoteIP(connId_);
}

void RMIReplier::send()
{
    server_.send(connId_, *this);
}

}}    // namespace

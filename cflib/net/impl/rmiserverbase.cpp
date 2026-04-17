/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "rmiserverbase.h"

#include <cflib/base.h>
#include <set>
#include <dirent.h>
#include <sys/stat.h>

#include <cflib/net/request.h>
#include <cflib/net/rmiservice.h>
#include <cflib/net/wscommmanager.h>
#include <cflib/serialize/impl/registerclass.h>
#include <cflib/util/log.h>
#include <cflib/util/util.h>

using namespace cflib::serialize;

USE_LOG(LogCat::Http)

namespace cflib::net::impl {

RMIServerBase::RMIServerBase(WSCommManagerBase & wsService) :
    ThreadVerify(&wsService),
    wsService_(wsService)
{
}

void RMIServerBase::registerService(RMIServiceBase & service)
{
    service.server_ = this;
    SerializeTypeInfo servInfo = service.getServiceInfo();
    serviceTypeInfos_ << servInfo;

    // register functions
    ServiceFunctions & sfs = services_[servInfo.typeName.toLower()];
    sfs.service = &service;
    uint i = 0;
    for (const auto & ti : servInfo.functions) {
        sfs.signatures[ti.signature()] = Pair<uint, uint>(++i, ti.hasReturnValues() ? 1u : 0u);
    }

    // register rsigs
    i = 0;
    for (const auto & ti : servInfo.cfSignals) {
        RSigBase & sig = *service.getCfSignal(++i);
        sig.server_ = this;
        sfs.signatures[ti.name] = Pair(i, 2);
    }
}

void RMIServerBase::send(uint connId, const ByteArray & data)
{
    if (!verifyThreadCall(&RMIServerBase::send, connId, data)) return;
    activeRequests_.erase(connId);
    wsService_.send(connId, data, true);
}

ByteArray RMIServerBase::getRemoteIP(uint connId)
{
    return wsService_.getRemoteIP(connId);
}

RMIServiceBase * RMIServerBase::checkServiceCall(serialize::BERDeserializer & deser, uint connId,
    uint & callNo, uint & type)
{
    String serviceName;
    String signature;
    deser >> serviceName >> signature;
    if (signature.isEmpty()) {
        logWarn("broken BER request from connection %1", connId);
        wsService_.close(connId, TCPConn::HardClosed);
        return 0;
    }

    ServiceFunctions sf = services_.value(serviceName);
    if (!sf.service) {
        logWarn("service %1 not found from connection %2", serviceName, connId);
        wsService_.close(connId, TCPConn::HardClosed);
        return 0;
    }

    Pair<uint, uint> method = sf.signatures.value(signature, Pair<uint, uint>(0u, 0u));
    if (method.first == 0) {
        logWarn("signature %1 of service %2 not found from connection %3", signature, serviceName, connId);
        wsService_.close(connId, TCPConn::HardClosed);
        return 0;
    }
    callNo = method.first;
    type   = method.second;

    if (type == 1) {
        if (activeRequests_.contains(connId)) {
            logWarn("two simultaneous requests from connection %1", connId);
            wsService_.close(connId, TCPConn::HardClosed);
            return 0;
        }
        activeRequests_ << connId;
    }
    return sf.service;
}

} // namespace

/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>
#include <cflib/net/rmiservice.h>
#include <cflib/serialize/serializeber.h>
#include <cflib/util/threadverify.h>

namespace cflib::net {

class Request;
class WSCommManagerBase;

namespace impl {

class RMIServerBase : public util::ThreadVerify
{
public:
    RMIServerBase(WSCommManagerBase & wsService);

    void registerService(RMIServiceBase & service);

    void send(uint connId, const ByteArray & data);
    ByteArray getRemoteIP(uint connId);

    template<typename C>
    void handleCall(const ByteArray & ba, const uint8 * data, int len, const C & connData, uint connDataId, uint connId)
    {
        if (!verifyThreadCall(&RMIServerBase::handleCall<C>, ba, data, len, connData, connDataId, connId)) return;

        serialize::BERDeserializer deser(ba, data, len);
        uint callNo;
        uint type;
        RMIServiceBase * serviceBase = checkServiceCall(deser, connId, callNo, type);
        if (!serviceBase) return;
        RMIService<C> * service = dynamic_cast<RMIService<C> *>(serviceBase);
        if (service) service    ->processRMIServiceCall(deser, callNo, type, connData, connDataId, connId);
        else         serviceBase->processRMIServiceCall(deser, callNo, type, connDataId, connId);
    }
    void handleCall(const ByteArray & ba, const uint8 * data, int len, uint connDataId, uint connId);

    template<typename C>
    void connDataChange(const C & connData, uint connDataId, const Set<uint> & connIds)
    {
        if (!verifyThreadCall(&RMIServerBase::connDataChange<C>, connData, connDataId, connIds)) return;

        for (auto & [name, sf] : services_) {
            RMIService<C> * service = dynamic_cast<RMIService<C> *>(sf.service);
            if (service) service->connDataChange(connData, connDataId, connIds);
        }
    }

    template<typename C>
    void connectionClosed(const C & connData, uint connDataId, uint connId, bool isLast)
    {
        if (!verifyThreadCall(&RMIServerBase::connectionClosed<C>, connData, connDataId, connId, isLast)) return;

        for (auto & [name, sf] : services_) {
            RMIServiceBase * serviceBase = sf.service;
            RMIService<C> * service = dynamic_cast<RMIService<C> *>(serviceBase);
            if (service) service    ->connectionClosed(connData, connDataId, connId, isLast);
            else         serviceBase->connectionClosed(connDataId, connId, isLast);
        }
    }
    void connectionClosed(uint connDataId, uint connId, bool isLast);

private:
    struct ServiceFunctions {
        ServiceFunctions() : service(0) {}
        RMIServiceBase * service;
        Map<String, Pair<uint, uint> > signatures;
    };

private:
    RMIServiceBase * checkServiceCall(serialize::BERDeserializer & deser, uint connId,
        uint & callNo, uint & type);

private:
    WSCommManagerBase & wsService_;
    Map<String, ServiceFunctions> services_;
    Set<uint> activeRequests_;
};

}} // namespace

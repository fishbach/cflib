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
#include <cflib/serialize/serializetypeinfo.h>
#include <cflib/util/threadverify.h>

namespace cflib { namespace net {

class Request;
class WSCommManagerBase;

namespace impl {

class RMIServerBase : public util::ThreadVerify
{
public:
    RMIServerBase(WSCommManagerBase & wsService);
    ~RMIServerBase();

    void registerService(RMIServiceBase & service);
    void exportTo(const String & dest) const;
    void handleRequest(const Request & request);
    void send(uint connId, const CFByteArray & data);
    CFByteArray getRemoteIP(uint connId);

    template<typename C>
    void handleCall(const CFByteArray & ba, const cfuint8 * data, int len, const C & connData, uint connDataId, uint connId)
    {
        if (!verifyThreadCall(&RMIServerBase::handleCall<C>, ba, data, len, connData, connDataId, connId)) return;

        serialize::BERDeserializer deser(ba, data, len);
        uint callNo;
        uint type;
        RMIServiceBase * serviceBase = checkServiceCall(deser, connId, callNo, type);
        if (!serviceBase) return;
        RMIService<C> * service = dynamic_cast<RMIService<C> *>(serviceBase);
        if (service) service    ->processRMIServiceCall(deser, callNo, type, connData, connDataId, connId);
        else         serviceBase->processRMIServiceCall(deser, callNo, type, connId);
    }

    template<typename C>
    void connDataChange(const C & connData, uint connDataId, const CFSet<uint> & connIds)
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
            else         serviceBase->connectionClosed(connId, isLast);
        }
    }

private:
    struct ClassInfoEl;
    class ClassInfos : public CFMap<String, ClassInfoEl *> {
        CF_DISABLE_COPY(ClassInfos)
    public:
        ClassInfos() {}
        ~ClassInfos() { for (auto & [k, v] : *this) delete v; }
    };
    struct ClassInfoEl {
        ClassInfos infos;
        cflib::serialize::SerializeTypeInfo ti;
    };
    struct ServiceFunctions {
        ServiceFunctions() : service(0) {}
        RMIServiceBase * service;
        CFMap<String, CFPair<uint, uint> > signatures;
    };

private:
    RMIServiceBase * checkServiceCall(serialize::BERDeserializer & deser, uint connId,
        uint & callNo, uint & type);
    void showServices(const Request & request, String path) const;
    void showClasses(const Request & request, String path) const;
    void classesToHTML(String & info, const ClassInfoEl & infoEl) const;
    String generateJSOrTS(const String & path) const;
    String generateJS(const serialize::SerializeTypeInfo & ti) const;
    String generateTS(const serialize::SerializeTypeInfo & ti) const;
    cflib::serialize::SerializeTypeInfo getTypeInfo(const String & path) const;
    String generateJSForClass(const cflib::serialize::SerializeTypeInfo & ti) const;
    String generateJSForService(const cflib::serialize::SerializeTypeInfo & ti) const;
    String generateTSForClass(const cflib::serialize::SerializeTypeInfo & ti) const;
    String generateTSForService(const cflib::serialize::SerializeTypeInfo & ti) const;
    CFSet<String> exportClass(const ClassInfoEl & cl, const String & path, const String & dest) const;
    void addClassInfo(const cflib::serialize::SerializeTypeInfo & ti);

private:
    WSCommManagerBase & wsService_;
    const CFRegex containerRE_;
    CFMap<String, ServiceFunctions> services_;
    ClassInfoEl classInfos_;
    CFSet<uint> activeRequests_;
};

}}}    // namespace

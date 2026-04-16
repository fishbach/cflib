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

namespace {

const String HTMLDocHeader =
    "<html><head>\n"
    "<title>CFLib API</title>\n"
    "<style type=\"text/css\">\n"
    "body { font-family: \"Verdana\"; }\n"
    "h2, h3, h4 { font-weight: normal; }\n"
    "</style>\n"
    "</head><body>\n"
    "<h2>CFLib API</h2>\n";

const String footer =
    "</body></html>\n";

template<typename List>
std::set<SerializeTypeInfo> getFunctionClassInfos(const List & functions);

std::set<SerializeTypeInfo> getClassInfos(const SerializeTypeInfo & ti)
{
    std::set<SerializeTypeInfo> retval;
    if (ti.type == SerializeTypeInfo::Class) {
        retval << ti;
        for (const auto & member : ti.members) {
            retval += getClassInfos(member.type);
        }
        retval += getFunctionClassInfos(ti.functions);
    }
    if (ti.type == SerializeTypeInfo::Class || ti.type == SerializeTypeInfo::Container) {
        for (const auto & base : ti.bases) {
            retval += getClassInfos(base);
        }
    }
    return retval;
}

template<typename List>
std::set<SerializeTypeInfo> getFunctionClassInfos(const List & functions)
{
    std::set<SerializeTypeInfo> retval;
    for (const auto & func : functions) {
        retval += getClassInfos(func.returnType);
        for (const auto & param : func.parameters) {
            retval += getClassInfos(param.type);
        }
    }
    return retval;
}

}

// ============================================================================

RMIServerBase::RMIServerBase(WSCommManagerBase & wsService) :
    ThreadVerify(&wsService),
    wsService_(wsService),
    containerRE_("^(.+)<(.+)>$")
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

    // add type infos
    for (const SerializeTypeInfo & ti :
        getFunctionClassInfos(servInfo.functions) + getFunctionClassInfos(servInfo.cfSignals))
    {
        addClassInfo(ti);

        // add all derived classes
        for (const SerializeTypeInfo & derivedClass : serialize::impl::RegisterClassBase::getAllSerializeTypeInfos()) {
            if (!derivedClass.isDerivedFrom(ti)) continue;

            for (const SerializeTypeInfo & memberClass : getClassInfos(derivedClass)) {
                addClassInfo(memberClass);
            }
        }
    }
}

void RMIServerBase::handleRequest(const Request & request)
{
    if (!verifyThreadCall(&RMIServerBase::handleRequest, request)) return;

    ByteArray path = request.getUri();
    {
        int p = path.indexOf('?');
        if (p != -1) path = path.left(p);
    }
    if (path.startsWith("/api/")) {
        request.addHeaderLine("Cache-Control: max-age=31536000");
        path.remove(0, 5);
        if      (path.startsWith("services")) showServices(request, path.mid(8));
        else if (path.startsWith("classes" )) showClasses (request, path.mid(7));
        else request.sendNotFound();
    } else if (path == "/api") {
        request.addHeaderLine("Cache-Control: max-age=31536000");
        String info = HTMLDocHeader;
        info <<
            "<ul>\n"
            "<li><a href=\"api/services\">services</a> - API Services Description</li>\n"
            "<li><a href=\"api/classes\">classes</a> - API Classes Description</li>\n"
            "</ul>\n";
        request.sendText(info << footer);
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

void RMIServerBase::showServices(const Request & request, String path) const
{
    String info = HTMLDocHeader;

    if (path.isEmpty()) {
        info <<
            "<h3>Services:</h3>\n"
            "<ul>\n";
        for (const auto & name : services_.keys()) {
            info
                << "<li><a href=\"services/" << name << "\">"
                << services_.value(name).service->getServiceInfo().typeName << "</a></li>\n";
        }
        info <<
            "</ul>\n";

        request.sendText(info << footer);
        return;
    }
    path.remove(0, 1);

    RMIServiceBase * srv = services_.value(path).service;
    if (!srv) return;
    SerializeTypeInfo ti = srv->getServiceInfo();

    info <<
        "<h3>Service: <b>" << ti.typeName << "</b></h3>\n"
        "JavaScript File: <a href=\"/js/services/" << path << ".mjs\">/js/services/" << path << ".mjs</a><br>\n"
        "<h4>Methods:</h4>\n"
        "<ul>\n";
    for (const auto & func : ti.functions) {
        info << "<li>" << func.signature(true).replace("<", "&lt;").replace(">", "&gt;") << "</li>\n";
    }
    info << "</ul>\n";

    if (!ti.cfSignals.isEmpty()) {
        info <<
            "<h4>Signals:</h4>\n"
            "<ul>\n";
        for (const auto & func : ti.cfSignals) {
            info << "<li>" << func.signature(true).replace("<", "&lt;").replace(">", "&gt;") << "</li>\n";
        }
        info << "</ul>\n";
    }

    request.sendText(info << footer);
}

void RMIServerBase::showClasses(const Request & request, String path) const
{
    String info = HTMLDocHeader;

    if (path.isEmpty()) {
        info <<
            "<h3>Classes:</h3>\n"
            "<ul>\n";
        classesToHTML(info, classInfos_);
        info << "</ul>\n";

        request.sendText(info << footer);
        return;
    }
    path.remove(0, 1);

    const SerializeTypeInfo ti = getTypeInfo(path);
    if (ti.getName().isEmpty()) return;

    info <<
        "<h3>Class: <b>" << ti.getName() << "</b></h3>\n"
        "JavaScript File: <a href=\"/js/" << path << ".mjs\">/js/" << path << ".mjs</a><br>\n"
        "<br>\n"
        "Base: ";
    if (ti.bases.isEmpty()) {
        info << "API.Base";
    } else {
        info << ti.bases[0].getName().replace("<", "&lt;").replace(">", "&gt;");
    }
    info <<
        "\n"
        "<h4>Members:</h4>\n"
        "<ul>\n";
    for (const auto & member : ti.members) {
        info
            << "<li>" << member.type.getName().replace("<", "&lt;").replace(">", "&gt;")
            << ' ' << member.name << "</li>\n";
    }
    info << "</ul>\n";

    request.sendText(info << footer);
}

void RMIServerBase::classesToHTML(String & info, const ClassInfoEl & infoEl) const
{
    for (const auto & ns : infoEl.infos.keys()) {
        const ClassInfoEl & el = *infoEl.infos.value(ns, (ClassInfoEl *)nullptr);
        if (!el.ti.getName().isEmpty()) {
            String path = el.ti.getName();
            path.replace("::", "/");
            info << "<li><a href=\"classes/" << path.toLower() << "\">" << el.ti.typeName.split("::").back() << "</a></li>\n";
        }
        if (!el.infos.isEmpty()) {
            info <<
                "<li>" << ns << ":</li>\n"
                "<ul>\n";
            classesToHTML(info, el);
            info << "</ul>\n";
        }
    }
}

SerializeTypeInfo RMIServerBase::getTypeInfo(const String & path) const
{
    auto srvIt = services_.find(path.mid(9));
    if (srvIt != services_.end()) {
        RMIServiceBase * srv = srvIt->second.service;
        if (srv) return srv->getServiceInfo();
    }

    const ClassInfoEl * ciEl = &classInfos_;
    for (const auto & ns : path.split('/')) {
        auto infoIt = ciEl->infos.find(ns);
        ciEl = (infoIt != ciEl->infos.end()) ? infoIt->second : nullptr;
        if (!ciEl) return SerializeTypeInfo();
    }
    return ciEl->ti;
}

void RMIServerBase::addClassInfo(const SerializeTypeInfo & ti)
{
    ClassInfoEl * ciEl = &classInfos_;
    for (const auto & ns : ti.getName().split("::")) {
        ClassInfoEl *& elRef = ciEl->infos[ns.toLower()];
        if (!elRef) elRef = new ClassInfoEl;
        ciEl = elRef;
    }
    ciEl->ti = ti;
}

} // namespace

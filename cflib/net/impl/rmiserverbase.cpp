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

USE_LOG(LogCat::Http)

using namespace cflib::serialize;

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

inline String formatClassnameForJS(const SerializeTypeInfo & ti)
{
    String retval = ti.getName();
    retval.replace("::", "__");
    return retval;
}

inline String formatMembernameForJS(const SerializeVariableTypeInfo & vti)
{
    if (vti.name.endsWith("_")) return vti.name.left(vti.name.length() - 1);
    return vti.name;
}

Set<String> getCustomTypes(const SerializeTypeInfo & ti)
{
    Set<String> types;
    if (ti.type == SerializeTypeInfo::Class) {
        types << ti.getName();
    } else if (ti.type == SerializeTypeInfo::Container) {
        for (const auto & base : ti.bases) {
            types += getCustomTypes(base);
        }
    }
    return types;
}

StringList getMemberTypes(const SerializeTypeInfo & ti)
{
    Set<String> types;
    for (const auto & base : ti.bases) {
        types += getCustomTypes(base);
    }
    for (const auto & member : ti.members) {
        types += getCustomTypes(member.type);
    }
    for (const auto & func : ti.functions) {
        types += getCustomTypes(func.returnType);
        for (const auto & param : func.parameters) {
            types += getCustomTypes(param.type);
        }
    }
    for (const auto & func : ti.cfSignals) {
        types += getCustomTypes(func.returnType);
        for (const auto & param : func.parameters) {
            types += getCustomTypes(param.type);
        }
    }

    StringList retval = cfSetValues(types);
    cfSort(retval);
    return retval;
}

String formatJSTypeConstruction(const SerializeTypeInfo & ti, const String & raw, bool useFactory)
{
    String js;
    if (ti.type == SerializeTypeInfo::Class) {
        if (useFactory) js << formatClassnameForJS(ti) << ".new(" << (raw == "null" ? "" : raw) << ")";
        else            js << "new " << formatClassnameForJS(ti) << "(" << (raw == "null" ? "" : raw) << ")";
    } else if (ti.type == SerializeTypeInfo::Container) {
        if (ti.typeName.startsWith("Pair<")) {
            if (raw == "null") js << "["
                << formatJSTypeConstruction(ti.bases[0], "null", useFactory) << ", "
                << formatJSTypeConstruction(ti.bases[1], "null", useFactory) << "]";
            else js << "(!" << raw << " ? ["
                << formatJSTypeConstruction(ti.bases[0], "null", useFactory) << ", "
                << formatJSTypeConstruction(ti.bases[1], "null", useFactory) << "] : ["
                << formatJSTypeConstruction(ti.bases[0], raw + "[0]", useFactory) << ", "
                << formatJSTypeConstruction(ti.bases[1], raw + "[1]", useFactory) << "])";
        } else if (ti.typeName.startsWith("List<")) {
            if (raw == "null") js << "[]";
            else js << "(" << raw << " || []).map(function(__e) { return "
                << formatJSTypeConstruction(ti.bases[0], "__e", useFactory) << "; })";
        } else if (ti.typeName.startsWith("Map<")) {
            if (raw == "null") js << "[]";
            else js << "(" << raw << " || []).map(function(__e) { return ["
                << formatJSTypeConstruction(ti.bases[0], "__e[0]", useFactory)
                << ", "
                << formatJSTypeConstruction(ti.bases[1], "__e[1]", useFactory) << "]; })";
        }
    } else if (ti.type == SerializeTypeInfo::Basic) {
        if (ti.typeName == "DateTime") {
            if (raw == "null") js << "null";
            else               js << "(!" << raw << " ? null : new Date(" << raw << "))";
        } else if (ti.typeName == "String") {
            if (raw == "null") js << "null";
            else               js << "(!" << raw << " && " << raw << " !== '' ? null : " << raw << ")";
        } else if (ti.typeName == "ByteArray") {
            if (raw == "null") js << "null";
            else               js << "(!" << raw << " ? null : " << raw << ")";
        } else if (ti.typeName.indexOf("int") != -1 || ti.typeName.indexOf("float") != -1) {
            if (raw == "null") js << "0";
            else               js << "(!" << raw << " ? 0 : " << raw << ")";
        } else if (ti.typeName == "tribool") {
            if (raw == "null") js << "undefined";
            else               js << "(" << raw << " === true || " << raw << " === 1 ? true : " << raw << " === false || " << raw << " === 2 ? false : undefined)";
        } else if (ti.typeName == "bool") {
            if (raw == "null") js << "false";
            else               js << "(" << raw << " ? true : false)";
        }
    }
    return js;
}

String getTSTypename(const SerializeTypeInfo & ti)
{
    String ts;
    if (ti.type == SerializeTypeInfo::Class) {
        ts << formatClassnameForJS(ti);
    } else if (ti.type == SerializeTypeInfo::Container) {
        if (ti.typeName.startsWith("Pair<")) {
            ts << "[" << getTSTypename(ti.bases[0]) << ", " << getTSTypename(ti.bases[1]) << "]";
        } else if (ti.typeName.startsWith("List<")) {
            ts << "Array<" << getTSTypename(ti.bases[0]) << ">";
        } else if (ti.typeName.startsWith("Map<")) {
            ts << "Array<[" << getTSTypename(ti.bases[0]) << ", " << getTSTypename(ti.bases[1]) << "]>";
        }
    } else if (ti.type == SerializeTypeInfo::Basic) {
        if (ti.typeName == "DateTime") {
            ts << "Date";
        } else if (ti.typeName == "String") {
            ts << "string";
        } else if (ti.typeName == "ByteArray") {
            ts << "Uint8Array";
        } else if (ti.typeName.indexOf("int") != -1 || ti.typeName.indexOf("float") != -1) {
            ts << "number";
        } else if (ti.typeName == "bool" || ti.typeName == "tribool") {
            ts << "boolean";
        }
    }
    return ts;
}

String getJSParameters(const List<SerializeVariableTypeInfo> & parameters, bool withType)
{
    String js;
    bool isFirst = true;
    int id = 0;
    for (const auto & p : parameters) {
        if (isFirst) isFirst = false;
        else js << ", ";
        if (p.name.isEmpty()) js << "__param_" << String::number(++id);
        else js << p.name;
        if (withType) js << ": " << getTSTypename(p.type);
    }
    return js;
}

String getJSParameters(const SerializeFunctionTypeInfo & func, bool withType)
{
    return getJSParameters(func.parameters, withType);
}

String getSerializeCode(const SerializeTypeInfo & ti, const String & name)
{
    String js;
    if (ti.type == SerializeTypeInfo::Class) {
        js << ".o(" << name << ")";
    } else if (ti.type == SerializeTypeInfo::Container) {
        if (ti.typeName.startsWith("Pair<")) {
            js    << ".p(" << name << ", function(__e, __S) { __S"
                << getSerializeCode(ti.bases[0], "__e[0]")
                << getSerializeCode(ti.bases[1], "__e[1]")
                << "; })";
        } else if (ti.typeName.startsWith("List<")) {
            js    << ".map(" << name << ", function(__e, __S) { __S"
                << getSerializeCode(ti.bases[0], "__e") << "; })";
        } else if (ti.typeName.startsWith("Map<")) {
            js    << ".map(" << name << ", function(__e, __S) { __S"
                << getSerializeCode(ti.bases[0], "__e[0]")
                << getSerializeCode(ti.bases[1], "__e[1]")
                << "; })";
        } else {
            logWarn("no code for Container type '%1'", ti.typeName);
        }
    } else if (ti.type == SerializeTypeInfo::Basic) {
        if (ti.typeName == "DateTime") {
            js << ".i(!" << name << " ? 0 : " << name << ".getTime())";
        } else if (ti.typeName == "String") {
            js << ".s(" << name << ")";
        } else if (ti.typeName == "ByteArray") {
            js << ".a(" << name << ")";
        } else if (ti.typeName.indexOf("int") != -1) {
            js << ".i(" << name << ")";
        } else if (ti.typeName == "float32") {
            js << ".f32(" << name << ")";
        } else if (ti.typeName == "float64") {
            js << ".f64(" << name << ")";
        } else if (ti.typeName == "tribool") {
            js << ".i(" << name << " === true || " << name << " === 1 ? 1 : " << name << " === false || " << name << " === 2 ? 2 : 0)";
        } else if (ti.typeName == "bool") {
            js << ".i(" << name << " ? 1 : 0)";
        } else {
            logWarn("no code for Basic type '%1'", ti.typeName);
        }
    } else {    // SerializeTypeInfo::Null
        logWarn("no code for type 'Null'");
    }
    return js;
}

String getDeserializeCode(const SerializeTypeInfo & ti, bool useFactory)
{
    String js;
    if (ti.type == SerializeTypeInfo::Class) {
        String cl = ti.getName();
        cl.replace("::", "__");
        if (useFactory) js << cl << ".new(__D.a())";
        else            js << "new " << cl << "(__D.a())";
    } else if (ti.type == SerializeTypeInfo::Container) {
        if (ti.typeName.startsWith("Pair<")) {
            js    << "(function(__data) { var __D = __ber.D(__data); return ["
                << getDeserializeCode(ti.bases[0], useFactory) << ", "
                << getDeserializeCode(ti.bases[1], useFactory) << "]; })(__D.a())";
        } else if (ti.typeName.startsWith("List<")) {
            js    << "__D.map(function(__D) { return "
                << getDeserializeCode(ti.bases[0], useFactory) << "; })";
        } else if (ti.typeName.startsWith("Map<")) {
            js    << "__D.map(function(__D) { return ["
                << getDeserializeCode(ti.bases[0], useFactory) << ", "
                << getDeserializeCode(ti.bases[1], useFactory) << "]; })";
        } else {
            logWarn("no code for Container type '%1'", ti.typeName);
        }
    } else if (ti.type == SerializeTypeInfo::Basic) {
        if (ti.typeName == "DateTime") {
            js << "(function() { var ti = __D.i(); return !ti ? null : new Date(ti); })()";
        } else if (ti.typeName == "String") {
            js << "__D.s()";
        } else if (ti.typeName == "ByteArray") {
            js << "__D.a()";
        } else if (ti.typeName.indexOf("int") != -1) {
            js << "__D.i()";
        } else if (ti.typeName == "float32") {
            js << "__D.f32()";
        } else if (ti.typeName == "float64") {
            js << "__D.f64()";
        } else if (ti.typeName == "tribool") {
            js << "(function() { var tb = __D.i(); return tb == 1 ? true : tb == 2 ? false : undefined; })()";
        } else if (ti.typeName == "bool") {
            js << "__D.i() ? true : false";
        } else {
            logWarn("no code for Basic type '%1'", ti.typeName);
        }
    } else {    // SerializeTypeInfo::Null
        logWarn("no code for type 'Null'");
    }
    return js;
}

String getSerializeJSParameters(const List<SerializeVariableTypeInfo> & parameters)
{
    String js;
    int id = 0;
    for (const auto & p : parameters) {
        String name = p.name;
        if (name.isEmpty()) name << "__param_" << String::number(++id);
        js << getSerializeCode(p.type, name);
    }
    return js;
}

String getSerializeJSParameters(const SerializeFunctionTypeInfo & func)
{
    return getSerializeJSParameters(func.parameters);
}

bool isDerivedFrom(const SerializeTypeInfo & derived, const SerializeTypeInfo & base)
{
    for (const SerializeTypeInfo & ti : derived.bases) {
        if (ti.getName() == base.getName()) return true;
        if (isDerivedFrom(ti, base)) return true;
    }
    return false;
}

}

// ============================================================================

RMIServerBase::RMIServerBase(WSCommManagerBase & wsService) :
    ThreadVerify("RMIServerBase", Worker),
    wsService_(wsService),
    containerRE_("^(.+)<(.+)>$")
{
}

RMIServerBase::~RMIServerBase()
{
    stopVerifyThread();
}

void RMIServerBase::registerService(RMIServiceBase & service)
{
    service.server_ = this;
    SerializeTypeInfo servInfo = service.getServiceInfo();

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
            if (!isDerivedFrom(derivedClass, ti)) continue;

            for (const SerializeTypeInfo & memberClass : getClassInfos(derivedClass)) {
                addClassInfo(memberClass);
            }
        }
    }
}

void RMIServerBase::exportTo(const String & dest) const
{
    // write services
    cflib::util::mkPath(dest + "/js/services");
    Set<String> files;
    for (const String & name : cfKeys(services_)) {
        for (const String & suffix : StringList{".mjs"/*, ".ts"*/}) {
            String file = name + suffix;
            files << file;
            String service = "services/" + file;
            String js = generateJSOrTS(service);
            cflib::util::writeFile(dest + "/js/" + service, js.toUtf8());
        }
    }

    // remove old
    {
        DIR * d = opendir((dest + "/js/services").c_str());
        if (d) {
            struct dirent * ent;
            while ((ent = readdir(d)) != nullptr) {
                String name(ent->d_name);
                if (name == "." || name == "..") continue;
                if (!cfContains(files, name)) cflib::util::removeFile(dest + "/js/services/" + name);
            }
            closedir(d);
        }
    }

    // write classes (recursive)
    files = exportClass(classInfos_, "", dest);

    // remove old - simplified: skip recursive cleanup for now
    CF_UNUSED(files);
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
        else if (path.startsWith("js/"     )) {
            String js = generateJSOrTS(path.mid(3));
            if (!js.isNull()) request.sendText(js, "application/javascript");
        } else request.sendNotFound();
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

    ServiceFunctions sf = cfMapValue(services_, serviceName);
    if (!sf.service) {
        logWarn("service %1 not found from connection %2", serviceName, connId);
        wsService_.close(connId, TCPConn::HardClosed);
        return 0;
    }

    Pair<uint, uint> method = cfMapValue(sf.signatures, signature, Pair<uint, uint>(0u, 0u));
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
        for (const auto & name : cfKeys(services_)) {
            info
                << "<li><a href=\"services/" << name << "\">"
                << cfMapValue(services_, name).service->getServiceInfo().typeName << "</a></li>\n";
        }
        info <<
            "</ul>\n";

        request.sendText(info << footer);
        return;
    }
    path.remove(0, 1);

    RMIServiceBase * srv = cfMapValue(services_, path).service;
    if (!srv) return;
    SerializeTypeInfo ti = srv->getServiceInfo();

    info <<
        "<h3>Service: <b>" << ti.typeName << "</b></h3>\n"
        "JavaScript File: <a href=\"/api/js/services/" << path << ".mjs\">/api/js/services/" << path << ".mjs</a><br>\n"
        "TypeScript File: <a href=\"/api/js/services/" << path << ".ts\">/api/js/services/" << path << ".ts</a>\n"
        "<h4>Methods:</h4>\n"
        "<ul>\n";
    for (const auto & func : ti.functions) {
        info << "<li>" << func.signature(true).replace("<", "&lt;").replace(">", "&gt;") << "</li>\n";
    }
    info << "</ul>\n";

    if (!ti.cfSignals.empty()) {
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
        "JavaScript File: <a href=\"/api/js/" << path << ".mjs\">/api/js/" << path << ".mjs</a><br>\n"
        "TypeScript File: <a href=\"/api/js/" << path << "dao.ts\">/api/js/" << path << "dao.ts</a><br>\n"
        "<br>\n"
        "Base: ";
    if (ti.bases.empty()) {
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
    for (const auto & ns : cfKeys(infoEl.infos)) {
        const ClassInfoEl & el = *cfMapValue(infoEl.infos, ns, (ClassInfoEl *)nullptr);
        if (!el.ti.getName().isEmpty()) {
            String path = el.ti.getName();
            path.replace("::", "/");
            info << "<li><a href=\"classes/" << path.toLower() << "\">" << el.ti.typeName.split("::").back() << "</a></li>\n";
        }
        if (!el.infos.empty()) {
            info <<
                "<li>" << ns << ":</li>\n"
                "<ul>\n";
            classesToHTML(info, el);
            info << "</ul>\n";
        }
    }
}

String RMIServerBase::generateJSOrTS(const String & path) const
{
    const bool isTS = path.endsWith(".ts");
    if (!path.endsWith(".mjs") && !isTS) return String();
    const SerializeTypeInfo ti = getTypeInfo(path.left(path.length() - (isTS && path.endsWith("dao.ts") ? 6 : 4)));
    if (ti.getName().isEmpty()) return String();

    String rv;
    rv <<
        "// ============================================================================\n"
        "// Generated by CFLib\n"
        "// ============================================================================\n"
        "\n";

    if (isTS) rv << generateTS(ti);
    else      rv << generateJS(ti);

    return rv;
}

String RMIServerBase::generateJS(const SerializeTypeInfo & ti) const
{
    const bool isService = !ti.functions.empty() || !ti.cfSignals.empty();

    String pathPrefix = "../";
    if (!isService) {
        for (int i = [&]() { int c = 0; cfsize_t p = 0; while ((p = ti.ns.str().find("::", p)) != std::string::npos) { ++c; p += 2; } return c; }() ; i > 0 ; --i) {
            pathPrefix << "../";
        }
    }

    String js;
    js << "import __ber from '" << pathPrefix << "cflib/net/ber.mjs';\n";
    if (isService) js << "import __rmi from '" << pathPrefix << "cflib/net/rmi.mjs';\n";
    else           js << "import __inherit from '" << pathPrefix << "cflib/util/inherit.mjs';\n";
    if (!ti.cfSignals.empty()) js << "import __RSig from '" << pathPrefix << "cflib/net/rsig.mjs';\n";
    for (String type : getMemberTypes(ti)) {
        String name = type;
        name.replace("::", "__");
        js << "import " << name;
        type.replace("::", "/");
        js << " from '" << pathPrefix << type.toLower() << ".mjs';\n";
    }
    js << "\n";

    if (isService) js << generateJSForService(ti);
    else           js << generateJSForClass(ti);

    return js;
}

String RMIServerBase::generateTS(const SerializeTypeInfo & ti) const
{
    const bool isService = !ti.functions.empty() || !ti.cfSignals.empty();
    const String cflibPath = ti.ns.startsWith("cflib::") && !isService ? "../../cflib/" : "../cflib/";

    String ts;
    ts <<
        "/* tslint:disable */\n"
        "\n";

    if (!ti.cfSignals.empty()) ts << "import {Observable} from 'rxjs/Observable';\n";

    ts <<
        "import {ber as __ber} from '" << cflibPath << "net/ber';\n";
    if (isService)               ts << "import {rmi as __rmi} from '" << cflibPath << "net/rmi';\n";
    else if (ti.bases.empty()) ts << "import {ModelBase as __modelBase} from '" << (ti.ns.startsWith("cflib::") ? "../" : "") << "../models/modelbase';\n";
    if (!ti.cfSignals.empty()) ts << "import {RemoteSignal as __RSig} from '" << cflibPath << "net/rsig';\n";
    for (String type : getMemberTypes(ti)) {
        String typePath = type.toLower();
        String typeName = type;
        typePath.replace("::", "/");
        typePath = CFRegex("^dao/").replaceAll(typePath, "models/");
        typePath = CFRegex("^cflib/dao/").replaceAll(typePath, "models/cflib/");
        typeName.replace("::", "__");
        if (type.contains("::")) type = type.mid(type.lastIndexOf("::") + 2);
        ts << "import {" << type << " as " << typeName << "} from '../" << typePath << "';\n";
    }
    ts << "\n";

    if (isService) ts << generateTSForService(ti);
    else           ts << generateTSForClass(ti);

    return ts;
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

String RMIServerBase::generateJSForClass(const SerializeTypeInfo & ti) const
{
    String base;
    if (!ti.bases.empty()) {
        base = ti.bases[0].getName();
        base.replace("::", "__");
    }

    String js;

    // JS namespace for debugging
    String nsPrefix;
    String typeName = ti.typeName;
    if (!ti.ns.isEmpty() || typeName.indexOf("::") != -1) {
        js << "var ";
        int i = 0;
        bool isFirst = true;
        for (const auto & ns : ti.ns.split("::")) {
            if (isFirst) {
                isFirst = false;
                js << ns << " = {";
            } else js << ns <<  ": {";
            nsPrefix << ns << '.';
            ++i;
        }
        StringList classNs = typeName.split("::");
        typeName = cfTakeLast(classNs);
        for (const auto & ns : classNs) {
            if (isFirst) {
                isFirst = false;
                js << ns << " = {";
            } else js << ns <<  ": {";
            nsPrefix << ns << '.';
            ++i;
        }
        while (--i >= 0) js << '}';
        js << ";\n"
            "\n";
    }

    if (nsPrefix.isEmpty()) js << "var ";
    js <<
        nsPrefix << typeName << " = function() { " <<
        nsPrefix << typeName << ".prototype.__init.apply(this, arguments); };\n"
        "__inherit.setBase(" << nsPrefix << typeName << ", ";
    if (base.isEmpty()) js << "__inherit.Base";
    else js << base;
    js << ");\n";
    if (ti.classId != 0) js << nsPrefix << typeName << ".__classId = " << String::number(ti.classId) << ";\n";
    js << nsPrefix << typeName << ".prototype.__init = function(param) {\n";
    if (base.isEmpty()) js << "    " << nsPrefix << typeName << ".__super.apply(this, arguments);\n";
    js <<
        "    if (param instanceof Uint8Array) {\n"
        "        var __D = __ber.D(param);\n"
        "        __D.n();\n";
    if (!base.isEmpty()) js << "        " << nsPrefix << typeName << ".__super.call(this, __D.a());\n";
    for (const auto & vti : ti.members) {
        js << "        this." << formatMembernameForJS(vti) << " = " << getDeserializeCode(vti.type, false) << ";\n";
    }
    js <<
        "    } else {\n";
    if (!base.isEmpty()) js <<"        " << nsPrefix << typeName << ".__super.apply(this, arguments);\n";
    js <<
        "        if (!param || typeof param != 'object') param = {};\n";
    for (const auto & vti : ti.members) {
        const String name = formatMembernameForJS(vti);
        js << "        this." << name << " = " << formatJSTypeConstruction(vti.type, "param." + name, false) << ";\n";
    }
    js <<
        "    }\n"
        "};\n"
        << nsPrefix << typeName << ".prototype.__serialize = function(__S) {\n"
        "    __S";
    if (ti.classId != 0) js << ".i(" << nsPrefix << typeName << ".__classId)";
    else                 js << ".n()";
    if (!base.isEmpty()) js << ".o(this, " << base << ".prototype.__serialize)";
    for (const auto & vti : ti.members) {
        js << getSerializeCode(vti.type, "this." + formatMembernameForJS(vti));
    }
    js << ".data();\n"
        "};\n"
        "export default " << nsPrefix << typeName << ";\n";
    return js;
}

String RMIServerBase::generateJSForService(const SerializeTypeInfo & ti) const
{
    String objName = ti.typeName;
    { char c = ti.typeName[0]; if (c >= 'A' && c <= 'Z') c += 32; objName[0] = c; }
    String js;
    js <<
        "var " << ti.typeName << " = function() {};\n"
        "var " << objName << " = new " << ti.typeName << "();\n"
        "\n";

    if (!ti.cfSignals.empty()) {
        js << objName << ".rsig = {\n";
        bool isFirst = true;
        for (const SerializeFunctionTypeInfo & func : ti.cfSignals) {
            if (isFirst) isFirst = false;
            else         js << ",\n";
            js << "    " << func.name << ": new __RSig(" << objName << ", '" << func.name << "', '" <<
                ti.typeName.toLower() << "', '" << func.name << "', function(";
            if (func.parameters.empty()) {
                js << ") { " << objName << ".rsig." << func.name << ".fire(); })";
            } else {
                js << "__D) { " << objName << ".rsig." << func.name << ".fire(";
                bool isFirst2 = true;
                for (const SerializeVariableTypeInfo & p : func.parameters) {
                    if (isFirst2) isFirst2 = false;
                    else          js << ", ";
                    js << getDeserializeCode(p.type, false);
                }
                js << "); })";
            }
        }
        js << "\n"
            "};\n"
            "\n";
    }

    for (const auto & func : ti.functions) {
        const bool hasRV = func.hasReturnValues();
        if (func.parameters.empty()) {
            js << objName << '.' << func.name << " = function(" << (hasRV ? "callback, context" : "") << ") {\n"
                "    __rmi.send" << (hasRV ? "Request" : "Async") << "(__ber.S().s('"
                << ti.typeName.toLower() << "').s('" << func.signature() << "').box(2)";
        } else {
            js << objName << '.' << func.name << " = function("
                << getJSParameters(func, false) << (hasRV ? ", callback, context" : "") << ") {\n"
                "    __rmi.send" << (hasRV ? "Request" : "Async") << "(__ber.S().s('"
                << ti.typeName.toLower() << "').s('" << func.signature() << "')" << getSerializeJSParameters(func) << ".box(2)";
        }

        if (!hasRV) {
            js << ");\n"
                "};\n"
                "\n";
            continue;
        }

        js <<
            ",\n"
            "        callback ? function(__data) {\n"
            "            var __D = __ber.D(__data);\n"
            "            callback.call(context";
        if (func.returnType.type != SerializeTypeInfo::Null) {
            js << ", " << getDeserializeCode(func.returnType, false);
        }
        for (const auto & p : func.parameters) {
            if (!p.isRef) continue;
            js << ", " << getDeserializeCode(p.type, false);
        }
        js <<
            ");\n"
            "        } : null);\n"
            "};\n"
            "\n";
    }

    js << "export default " << objName << ";\n";
    return js;
}

String RMIServerBase::generateTSForClass(const SerializeTypeInfo & ti) const
{
    String base;
    if (!ti.bases.empty()) {
        base = ti.bases[0].getName();
        base.replace("::", "__");
    }

    String typeName = ti.typeName;
    if (typeName.contains("::")) typeName = typeName.mid(typeName.lastIndexOf("::") + 2);

    String ts;
    ts << "export abstract class " << typeName << "Dao extends " << (!base.isEmpty() ? base : "__modelBase") << " {\n"
        "\n";

    if (ti.classId != 0) ts << "    static __classId: number = " << String::number(ti.classId) << ";\n";
    if (!ti.members.empty()) {
        for (const auto & vti : ti.members) {
            ts << "    " << formatMembernameForJS(vti) << ": " << getTSTypename(vti.type) << ";\n";
        }
    }
    if (ti.classId != 0 || !ti.members.empty()) ts << "\n";

    ts <<
        "    constructor(param?) {\n";
    if (base.isEmpty()) ts << "        super(param);\n";
    ts <<
        "        if (param instanceof Uint8Array) {\n"
        "            var __D = __ber.D(param);\n"
        "            __D.n();\n";
    if (!base.isEmpty()) ts << "            super(__D.a());\n";
    for (const auto & vti : ti.members) {
        ts << "            this." << formatMembernameForJS(vti) << " = " << getDeserializeCode(vti.type, true) << ";\n";
    }

    ts <<
        "        } else {\n";
    if (!base.isEmpty()) ts << "            super(param);\n";
    ts <<
        "            if (!param || typeof param != 'object') param = {};\n";

    for (const auto & vti : ti.members) {
        const String name = formatMembernameForJS(vti);
        ts << "            this." << name << " = " << formatJSTypeConstruction(vti.type, "param." + name, true) << ";\n";
    }
    ts <<
        "        }\n"
        "    }\n"
        "\n"
        "    protected __serialize(__S): void {\n"
        "        __S.";
    if (ti.classId != 0) ts << "i(" << typeName << "Dao.__classId)";
    else                 ts << "n()";
    if (!base.isEmpty()) ts << ".o(this, super.__serialize)";
    for (const auto & vti : ti.members) {
        ts << getSerializeCode(vti.type, "this." + formatMembernameForJS(vti));
    }
    ts << ".data();\n"
        "    }\n"
        "\n";

    ts <<
        "    static new(model): any {\n";
    if (ti.classId != 0) ts << "        __ber.ClassRegistry.set(" << typeName << "Dao.__classId, model);\n";
    ts <<
        "        return __ber.dynamicCreate(model);\n"
        "    }\n"
        "\n"
        "}\n";

    return ts;
}

String RMIServerBase::generateTSForService(const SerializeTypeInfo & ti) const
{
    String objName = ti.typeName;
    { char c = ti.typeName[0]; if (c >= 'A' && c <= 'Z') c += 32; objName[0] = c; }
    String ts;

    for (const auto & func : ti.cfSignals) {
        String funcTypename = func.name;
        { char c = func.name[0]; if (c >= 'a' && c <= 'z') c -= 32; funcTypename[0] = c; }
        ts << "interface __" << funcTypename << " {\n"
            "    register(" << getJSParameters(func.registerParameters, true) << "): Observable<";
        if (func.parameters.size() > 1) ts << "[";
        bool isFirst = true;
        for (const auto & p : func.parameters) {
            if (isFirst) isFirst = false;
            else         ts << ", ";
            ts << getTSTypename(p.type);
        }
        if (func.parameters.size() > 1) ts << "]";
        ts << ">;\n"
            "}\n"
            "\n";
    }

    ts <<
        "export class " << ti.typeName << " {\n"
        "\n";

    if (!ti.cfSignals.empty()) {
        ts << "    rsig: {\n";
        bool isFirst = true;
        for (const auto & func : ti.cfSignals) {
            if (isFirst) isFirst = false;
            else         ts << ",\n";
            String funcTypename = func.name;
            { char c = func.name[0]; if (c >= 'a' && c <= 'z') c -= 32; funcTypename[0] = c; }
            ts << "        " << func.name << ": __" << funcTypename;
        }
        ts << "\n"
            "    };\n"
            "\n"
            "    constructor() {\n"
            "        this.rsig = {\n";

        isFirst = true;
        for (const auto & func : ti.cfSignals) {
            if (isFirst) isFirst = false;
            else         ts << ",\n";

            ts << "            " << func.name << ": new __RSig(\n"
                "                '" << ti.typeName.toLower() << "', '" << func.name << "',\n"
                "                function(";
            if (func.registerParameters.empty()) {
                ts << ") {},\n";
            } else {
                ts << "__S, " << getJSParameters(func.registerParameters, false) << ") {\n"
                    "                    __S" << getSerializeJSParameters(func.registerParameters) << ";\n"
                    "                },\n";
            }

            ts << "                function(__data) {\n"
                "                    var __D = __ber.D(__data);\n"
                "                    return ";
            if (func.parameters.size() > 1) ts << "[";

            bool isFirst = true;
            for (const auto & p : func.parameters) {
                if (isFirst) isFirst = false;
                else          ts << ", ";
                ts << getDeserializeCode(p.type, true);
            }

            if (func.parameters.size() > 1) ts << "]";
            ts << ";\n"
                "                }\n"
                "            )";
        }

        ts << "\n"
            "        };\n"
            "    }\n"
            "\n";
    }

    for (const auto & func : ti.functions) {
        const uint rvCount = func.returnValueCount();

        ts << "    " << func.name << "(" << getJSParameters(func, true) << "): ";
        if (rvCount == 0) {
            ts << "void";
        } else {
            ts << "Promise<";
            if (rvCount > 1) ts << "[";
            bool isFirst = true;
            if (func.returnType.type != SerializeTypeInfo::Null) {
                ts << getTSTypename(func.returnType);
                isFirst = false;
            }
            for (const auto & p : func.parameters) {
                if (!p.isRef) continue;
                if (isFirst) isFirst = false;
                else         ts << ", ";
                ts << getTSTypename(p.type);
            }
            if (rvCount > 1) ts << "]";
            ts << ">";
        }
        ts << " {\n"
            "        " << (rvCount > 0 ? "return " : "") << "__rmi.send" << (rvCount > 0 ? "Request" : "Async") << "(__ber.S().s('"
            << ti.typeName.toLower() << "').s('" << func.signature() << "')" << getSerializeJSParameters(func) << ".box(2)";

        if (rvCount == 0) {
            ts << ");\n"
                "    }\n"
                "\n";
            continue;
        }

        ts <<
            ",\n"
            "            function(__data) {\n"
            "                var __D = __ber.D(__data);\n"
            "                return ";
        if (rvCount > 1) ts << "[";
        bool isFirst = true;
        if (func.returnType.type != SerializeTypeInfo::Null) {
            ts << getDeserializeCode(func.returnType, true);
            isFirst = false;
        }
        for (const auto & p : func.parameters) {
            if (!p.isRef) continue;
            if (isFirst) isFirst = false;
            else         ts << ", ";
            ts << getDeserializeCode(p.type, true);
        }
        if (rvCount > 1) ts << "]";
        ts << ";\n"
            "            });\n"
            "    }\n"
            "\n";
    }

    ts <<
        "}\n"
        "\n"
        "export const " << objName << ": " << ti.typeName << " = new " << ti.typeName << "();";

    return ts;
}

Set<String> RMIServerBase::exportClass(const ClassInfoEl & cl, const String & path, const String & dest) const
{
    Set<String> rv;
    if (cl.infos.empty()) {
        if (path.isEmpty()) return rv;
        for (const String & suffix : StringList{".mjs"/*, "dao.ts"*/}) {
            String cl = path + suffix;
            rv << cl;
            String js = generateJSOrTS(cl);
            cflib::util::writeFile(dest + "/js/" + cl, js.toUtf8());
        }
    } else {
        cflib::util::mkPath(dest + "/js/" + path);
        String p = path;
        if (!path.isEmpty()) p += '/';
        for (const auto & [ns, elPtr] : cl.infos) {
            rv += exportClass(*elPtr, p + ns, dest);
        }
    }
    return rv;
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

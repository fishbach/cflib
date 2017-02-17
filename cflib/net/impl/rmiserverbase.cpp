/* Copyright (C) 2013-2016 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * cflib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * cflib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cflib. If not, see <http://www.gnu.org/licenses/>.
 */

#include "rmiserverbase.h"

#include <cflib/net/request.h>
#include <cflib/net/rmiservice.h>
#include <cflib/net/wscommmanager.h>
#include <cflib/serialize/impl/registerclass.h>
#include <cflib/util/log.h>
#include <cflib/util/util.h>

USE_LOG(LogCat::Http)

using namespace cflib::serialize;

namespace cflib { namespace net { namespace impl {

namespace {

const QString HTMLDocHeader =
	"<html><head>\n"
	"<title>CFLib API</title>\n"
	"<style type=\"text/css\">\n"
	"body { font-family: \"Verdana\"; }\n"
	"h2, h3, h4 { font-weight: normal; }\n"
	"</style>\n"
	"</head><body>\n"
	"<h2>CFLib API</h2>\n";

const QString footer =
	"</body></html>\n";

QSet<SerializeTypeInfo> getFunctionClassInfos(const QList<SerializeFunctionTypeInfo> & functions);

QSet<SerializeTypeInfo> getClassInfos(const SerializeTypeInfo & ti)
{
	QSet<SerializeTypeInfo> retval;
	if (ti.type == SerializeTypeInfo::Class) {
		retval << ti;
		foreach (const SerializeVariableTypeInfo & member, ti.members) {
			retval += getClassInfos(member.type);
		}
		retval += getFunctionClassInfos(ti.functions);
	}
	if (ti.type == SerializeTypeInfo::Class || ti.type == SerializeTypeInfo::Container) {
		foreach (const SerializeTypeInfo & base, ti.bases) {
			retval += getClassInfos(base);
		}
	}
	return retval;
}

QSet<SerializeTypeInfo> getFunctionClassInfos(const QList<SerializeFunctionTypeInfo> & functions)
{
	QSet<SerializeTypeInfo> retval;
	foreach (const SerializeFunctionTypeInfo & func, functions) {
		retval += getClassInfos(func.returnType);
		foreach (const SerializeVariableTypeInfo & param, func.parameters) {
			retval += getClassInfos(param.type);
		}
	}
	return retval;
}

inline QString formatClassnameForJS(const SerializeTypeInfo & ti)
{
	QString retval = ti.getName();
	retval.replace("::", "__");
	return retval;
}

inline QString formatMembernameForJS(const SerializeVariableTypeInfo & vti)
{
	if (vti.name.endsWith('_')) return vti.name.left(vti.name.length() - 1);
	return vti.name;
}

QSet<QString> getCustomTypes(const SerializeTypeInfo & ti)
{
	QSet<QString> types;
	if (ti.type == SerializeTypeInfo::Class) {
		types << ti.getName();
	} else if (ti.type == SerializeTypeInfo::Container) {
		foreach (const SerializeTypeInfo & base, ti.bases) {
			types += getCustomTypes(base);
		}
	}
	return types;
}

QStringList getMemberTypes(const SerializeTypeInfo & ti)
{
	QSet<QString> types;
	foreach (const SerializeTypeInfo & base, ti.bases) {
		types += getCustomTypes(base);
	}
	foreach (const SerializeVariableTypeInfo & member, ti.members) {
		types += getCustomTypes(member.type);
	}
	foreach (const SerializeFunctionTypeInfo & func, ti.functions) {
		types += getCustomTypes(func.returnType);
		foreach (const SerializeVariableTypeInfo & param, func.parameters) {
			types += getCustomTypes(param.type);
		}
	}
	foreach (const SerializeFunctionTypeInfo & func, ti.cfSignals) {
		types += getCustomTypes(func.returnType);
		foreach (const SerializeVariableTypeInfo & param, func.parameters) {
			types += getCustomTypes(param.type);
		}
	}

	QStringList retval = types.toList();
	retval.sort();
	return retval;
}

QString formatJSTypeConstruction(const SerializeTypeInfo & ti, const QString & raw, bool useFactory)
{
	QString js;
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

QString getTSTypename(const SerializeTypeInfo & ti)
{
	QString ts;
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

QString getJSFunctionName(const QRegularExpression & containerRE, const SerializeFunctionTypeInfo & func)
{
	QString js = func.name;
	js << '_';
	foreach (const SerializeVariableTypeInfo & p, func.parameters) {
		const QRegularExpressionMatch m = containerRE.match(p.type.typeName);
		if (m.hasMatch()) {
			js << m.captured(2) << m.captured(1);
		} else {
			js << p.type.typeName;
		}
	}
	js.replace(',', "");
	return js;
}

QString getJSParameters(const QList<SerializeVariableTypeInfo> & parameters, bool withType)
{
	QString js;
	bool isFirst = true;
	int id = 0;
	foreach (const SerializeVariableTypeInfo & p, parameters) {
		if (isFirst) isFirst = false;
		else js << ", ";
		if (p.name.isEmpty()) js << "__param_" << QString::number(++id);
		else js << p.name;
		if (withType) js << ": " << getTSTypename(p.type);
	}
	return js;
}

QString getJSParameters(const SerializeFunctionTypeInfo & func, bool withType)
{
	return getJSParameters(func.parameters, withType);
}

QString getSerializeCode(const SerializeTypeInfo & ti, const QString & name)
{
	QString js;
	if (ti.type == SerializeTypeInfo::Class) {
		js << ".o(" << name << ")";
	} else if (ti.type == SerializeTypeInfo::Container) {
		if (ti.typeName.startsWith("Pair<")) {
			js	<< ".p(" << name << ", function(__e, __S) { __S"
				<< getSerializeCode(ti.bases[0], "__e[0]")
				<< getSerializeCode(ti.bases[1], "__e[1]")
				<< "; })";
		} else if (ti.typeName.startsWith("List<")) {
			js	<< ".map(" << name << ", function(__e, __S) { __S"
				<< getSerializeCode(ti.bases[0], "__e") << "; })";
		} else if (ti.typeName.startsWith("Map<")) {
			js	<< ".map(" << name << ", function(__e, __S) { __S"
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
	} else {	// SerializeTypeInfo::Null
		logWarn("no code for type 'Null'");
	}
	return js;
}

QString getDeserializeCode(const SerializeTypeInfo & ti, bool useFactory)
{
	QString js;
	if (ti.type == SerializeTypeInfo::Class) {
		QString cl = ti.getName();
		cl.replace("::", "__");
		if (useFactory) js << cl << ".new(__D.a())";
		else            js << "new " << cl << "(__D.a())";
	} else if (ti.type == SerializeTypeInfo::Container) {
		if (ti.typeName.startsWith("Pair<")) {
			js	<< "(function(__data) { var __D = __ber.D(__data); return ["
				<< getDeserializeCode(ti.bases[0], useFactory) << ", "
				<< getDeserializeCode(ti.bases[1], useFactory) << "]; })(__D.a())";
		} else if (ti.typeName.startsWith("List<")) {
			js	<< "__D.map(function(__D) { return "
				<< getDeserializeCode(ti.bases[0], useFactory) << "; })";
		} else if (ti.typeName.startsWith("Map<")) {
			js	<< "__D.map(function(__D) { return ["
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
	} else {	// SerializeTypeInfo::Null
		logWarn("no code for type 'Null'");
	}
	return js;
}

QString getSerializeJSParameters(const QList<SerializeVariableTypeInfo> & parameters)
{
	QString js;
	int id = 0;
	foreach (const SerializeVariableTypeInfo & p, parameters) {
		QString name = p.name;
		if (name.isEmpty()) name << "__param_" << QString::number(++id);
		js << getSerializeCode(p.type, name);
	}
	return js;
}

QString getSerializeJSParameters(const SerializeFunctionTypeInfo & func)
{
	return getSerializeJSParameters(func.parameters);
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

	ServiceFunctions & sfs = services_[servInfo.typeName.toLower()];
	sfs.service = &service;
	uint i = 0;
	foreach (const SerializeFunctionTypeInfo & ti, servInfo.functions) {
		sfs.signatures[ti.signature()] = qMakePair(++i, ti.hasReturnValues() ? 1 : 0);
	}

	i = 0;
	foreach (const SerializeFunctionTypeInfo & ti, servInfo.cfSignals) {
		RSigBase & sig = *service.getCfSignal(++i);
		sig.server_ = this;
		sfs.signatures[ti.name] = qMakePair(i, 2);
	}

	for (const SerializeTypeInfo & ti :
		getFunctionClassInfos(servInfo.functions) + getFunctionClassInfos(servInfo.cfSignals))
	{
		ClassInfoEl * ciEl = &classInfos_;
		foreach (const QString & ns, ti.getName().split("::")) {
			ClassInfoEl *& elRef = ciEl->infos[ns.toLower()];
			if (!elRef) elRef = new ClassInfoEl;
			ciEl = elRef;
		}
		ciEl->ti = ti;

		// add all derived classes
		for (const SerializeTypeInfo & derived : serialize::impl::RegisterClassBase::getAllSerializeTypeInfos()) {
			// check all bases
			bool found = false;
			for (const SerializeTypeInfo & base : derived.bases) {
				if (base.getName() == ti.getName()) {
					found = true;
					break;
				}
			}
			if (!found) continue;

			ciEl = &classInfos_;
			foreach (const QString & ns, derived.getName().split("::")) {
				ClassInfoEl *& elRef = ciEl->infos[ns.toLower()];
				if (!elRef) elRef = new ClassInfoEl;
				ciEl = elRef;
			}
			ciEl->ti = derived;
		}
	}
}

void RMIServerBase::exportTo(const QString & dest) const
{
	// write services
	QDir().mkpath(dest + "/js/services");
	foreach (const QString & name, services_.keys()) {
		for (const QString & suffix : QStringList{".js", "api.ts"}) {
			QString service = "services/" + name + suffix;
			QString js = generateJSOrTS(service);
			QFile f(dest + "/js/" + service);
			f.open(QFile::WriteOnly | QFile::Truncate);
			f.write(js.toUtf8());
		}
	}

	// write classes (recursive)
	exportClass(classInfos_, "", dest);
}

void RMIServerBase::handleRequest(const Request & request)
{
	if (!verifyThreadCall(&RMIServerBase::handleRequest, request)) return;

	QByteArray path = request.getUri();
	{
		int p = path.indexOf('?');
		if (p != -1) path = path.left(p);
	}
	if (path.startsWith("/api/")) {
		path.remove(0, 5);
		if (path.startsWith("services")) showServices(request, path.mid(8));
		else if (path.startsWith("classes"))  showClasses(request, path.mid(7));
		else request.sendNotFound();
	} else {
		if (path.startsWith("/js/")) {
			QString js = generateJSOrTS(path.mid(4));
			if (!js.isNull()) request.sendText(js, "application/javascript");
		} else if (path == "/api") {
			QString info = HTMLDocHeader;
			info <<
				"<ul>\n"
				"<li><a href=\"api/services\">services</a> - API Services Description</li>\n"
				"<li><a href=\"api/classes\">classes</a> - API Classes Description</li>\n"
				"</ul>\n";
			request.sendText(info << footer);
		}
	}
}

void RMIServerBase::send(uint connId, const QByteArray & data)
{
	if (!verifyThreadCall(&RMIServerBase::send, connId, data)) return;
	activeRequests_.remove(connId);
	wsService_.send(connId, data, true);
}

QByteArray RMIServerBase::getRemoteIP(uint connId)
{
	return wsService_.getRemoteIP(connId);
}

RMIServiceBase * RMIServerBase::checkServiceCall(serialize::BERDeserializer & deser, uint connId,
	uint & callNo, uint & type)
{
	QString serviceName;
	QString signature;
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

	QPair<uint, uint> method = sf.signatures.value(signature);
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

void RMIServerBase::showServices(const Request & request, QString path) const
{
	QString info = HTMLDocHeader;

	if (path.isEmpty()) {
		info <<
			"<h3>Services:</h3>\n"
			"<ul>\n";
		foreach (const QString & name, services_.keys()) {
			info
				<< "<li><a href=\"services/" << name << "\">"
				<< services_[name].service->getServiceInfo().typeName << "</a></li>\n";
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
		"JavaScript File: <a href=\"/js/services/" << path << ".js\">/js/services/" << path << ".js</a><br>\n"
		"TypeScript File: <a href=\"/js/services/" << path << "api.ts\">/js/services/" << path << "api.ts</a>\n"
		"<h4>Methods:</h4>\n"
		"<ul>\n";
	foreach (const SerializeFunctionTypeInfo & func, ti.functions) {
		info << "<li>" << func.signature(true).replace('<', "&lt;").replace('>', "&gt;") << "</li>\n";
	}
	info << "</ul>\n";

	if (!ti.cfSignals.isEmpty()) {
		info <<
			"<h4>Signals:</h4>\n"
			"<ul>\n";
		foreach (const SerializeFunctionTypeInfo & func, ti.cfSignals) {
			info << "<li>" << func.signature(true).replace('<', "&lt;").replace('>', "&gt;") << "</li>\n";
		}
		info << "</ul>\n";
	}

	request.sendText(info << footer);
}

void RMIServerBase::showClasses(const Request & request, QString path) const
{
	QString info = HTMLDocHeader;

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
		"JavaScript File: <a href=\"/js/" << path << ".js\">/js/" << path << ".js</a><br>\n"
		"TypeScript File: <a href=\"/js/" << path << "dao.ts\">/js/" << path << "dao.ts</a><br>\n"
		"<br>\n"
		"Base: ";
	if (ti.bases.isEmpty()) {
		info << "API.Base";
	} else {
		info << ti.bases[0].getName().replace('<', "&lt;").replace('>', "&gt;");
	}
	info <<
		"\n"
		"<h4>Members:</h4>\n"
		"<ul>\n";
	foreach (const SerializeVariableTypeInfo & member, ti.members) {
		info
			<< "<li>" << member.type.getName().replace('<', "&lt;").replace('>', "&gt;")
			<< ' ' << member.name << "</li>\n";
	}
	info << "</ul>\n";

	request.sendText(info << footer);
}

void RMIServerBase::classesToHTML(QString & info, const ClassInfoEl & infoEl) const
{
	foreach (const QString & ns, infoEl.infos.keys()) {
		const ClassInfoEl & el = *infoEl.infos.value(ns, nullptr);
		if (!el.ti.getName().isEmpty()) {
			QString path = el.ti.getName();
			path.replace("::", "/");
			info << "<li><a href=\"classes/" << path.toLower() << "\">" << el.ti.typeName.split("::").last() << "</a></li>\n";
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

QString RMIServerBase::generateJSOrTS(const QString & path) const
{
	const bool isTS = path.endsWith(".ts");
	if (!path.endsWith(".js") && !isTS) return QString();
	const SerializeTypeInfo ti = getTypeInfo(path.left(path.length() - (isTS ? 6 : 3)));
	if (ti.getName().isEmpty()) return QString();

	QString rv;
	rv <<
		"// ============================================================================\n"
		"// Generated by CFLib\n"
		"// ============================================================================\n"
		"\n";

	if (isTS) rv << generateTS(ti);
	else      rv << generateJS(ti);

	return rv;
}

QString RMIServerBase::generateJS(const SerializeTypeInfo & ti) const
{
	const bool isService = !ti.functions.isEmpty();

	QString js;
	js <<
		"define(['cflib/net/ber', 'cflib/" << (isService ? "net/rmi'" : "util/inherit'") <<
		(ti.cfSignals.isEmpty() ? "" : ", 'cflib/net/rsig'");
	foreach (QString type, getMemberTypes(ti)) {
		type.replace("::", "/");
		js << ", '" << type.toLower() << '\'';
	}
	js << "], function(__ber, " << (isService ? "__rmi" : "__inherit") <<
		(ti.cfSignals.isEmpty() ? "" : ", __RSig");
	foreach (QString type, getMemberTypes(ti)) {
		type.replace("::", "__");
		js << ", " << type;
	}
	js << ") {\n"
		"\n";

	if (isService) js << generateJSForService(ti);
	else           js << generateJSForClass(ti);

	js <<
		"\n"
		"});\n";
	return js;
}

QString RMIServerBase::generateTS(const SerializeTypeInfo & ti) const
{
	const bool isService = !ti.functions.isEmpty();
	const QString cflibPath = ti.ns.startsWith("cflib::") && !isService ? "../../cflib/" : "../cflib/";

	QString ts;
	ts <<
		"/* tslint:disable */\n"
		"\n";

	if (!ti.cfSignals.isEmpty()) ts << "import {Observable} from 'rxjs/Observable';\n";

	ts <<
		"import {ber as __ber} from '" << cflibPath << "net/ber';\n";
	if (isService)               ts << "import {rmi as __rmi} from '" << cflibPath << "net/rmi';\n";
	else if (ti.bases.isEmpty()) ts << "import {ModelBase as __modelBase} from '" << (ti.ns.startsWith("cflib::") ? "../" : "") << "../models/modelbase';\n";
	if (!ti.cfSignals.isEmpty()) ts << "import {RemoteSignal as __RSig} from '" << cflibPath << "net/rsig';\n";
	foreach (QString type, getMemberTypes(ti)) {
		QString typePath = type.toLower();
		QString typeName = type;
		typePath.replace("::", "/").replace(QRegExp("^dao/"), "models/").replace(QRegExp("^cflib/dao/"), "models/cflib/");
		typeName.replace("::", "__");
		if (type.contains("::")) type = type.mid(type.lastIndexOf("::") + 2);
		ts << "import {" << type << " as " << typeName << "} from '../" << typePath << "';\n";
	}
	ts << "\n";

	if (isService) ts << generateTSForService(ti);
	else           ts << generateTSForClass(ti);

	return ts;
}

SerializeTypeInfo RMIServerBase::getTypeInfo(const QString & path) const
{
	RMIServiceBase * srv = services_[path.mid(9)].service;
	if (srv) return srv->getServiceInfo();

	const ClassInfoEl * ciEl = &classInfos_;
	foreach (const QString & ns, path.split('/')) {
		ciEl = ciEl->infos[ns];
		if (!ciEl) return SerializeTypeInfo();
	}
	return ciEl->ti;
}

QString RMIServerBase::generateJSForClass(const SerializeTypeInfo & ti) const
{
	QString base;
	if (!ti.bases.isEmpty()) {
		base = ti.bases[0].getName();
		base.replace("::", "__");
	}

	QString js;

	// JS namespace for debugging
	QString nsPrefix;
	QString typeName = ti.typeName;
	if (!ti.ns.isEmpty() || typeName.indexOf("::") != -1) {
		js << "var ";
		int i = 0;
		bool isFirst = true;
		foreach (const QString & ns, ti.ns.split("::")) {
			if (isFirst) {
				isFirst = false;
				js << ns << " = {";
			} else js << ns <<  ": {";
			nsPrefix << ns << '.';
			++i;
		}
		QStringList classNs = typeName.split("::");
		typeName = classNs.takeLast();
		foreach (const QString & ns, classNs) {
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
	js	<< ");\n"
		<< nsPrefix << typeName << ".prototype.__init = function(param) {\n";
	if (base.isEmpty()) js << "\t" << nsPrefix << typeName << ".__super.apply(this, arguments);\n";
	js	<< "\tif (param instanceof Uint8Array) {\n"
		"\t\tvar __D = __ber.D(param);\n";
	if (!base.isEmpty()) js << "\t\t" << nsPrefix << typeName << ".__super.call(this, __D.a());\n";
	foreach (const SerializeVariableTypeInfo & vti, ti.members) {
		js << "\t\tthis." << formatMembernameForJS(vti) << " = " << getDeserializeCode(vti.type, false) << ";\n";
	}
	js <<
		"\t} else {\n";
	if (!base.isEmpty()) js <<"\t\t" << nsPrefix << typeName << ".__super.apply(this, arguments);\n";
	js <<
		"\t\tif (!param || typeof param != 'object') param = {};\n";
	foreach (const SerializeVariableTypeInfo & vti, ti.members) {
		const QString name = formatMembernameForJS(vti);
		js << "\t\tthis." << name << " = " << formatJSTypeConstruction(vti.type, "param." + name, false) << ";\n";
	}
	js <<
		"\t}\n"
		"};\n"
		<< nsPrefix << typeName << ".prototype.__serialize = function(__S) {\n"
		"\t__S";
	if (!base.isEmpty()) js << ".o(this, " << base << ".prototype.__serialize)";
	foreach (const SerializeVariableTypeInfo & vti, ti.members) {
		js << getSerializeCode(vti.type, "this." + formatMembernameForJS(vti));
	}
	js << ".data();\n"
		"};\n"
		"return " << nsPrefix << typeName << ";\n";
	return js;
}

QString RMIServerBase::generateJSForService(const SerializeTypeInfo & ti) const
{
	QString objName = ti.typeName;
	objName[0] = ti.typeName[0].toLower();
	QString js;
	js <<
		"var " << ti.typeName << " = function() {};\n"
		"var " << objName << " = new " << ti.typeName << "();\n"
		"\n";

	if (!ti.cfSignals.isEmpty()) {
		js << objName << ".rsig = {\n";
		bool isFirst = true;
		foreach (const SerializeFunctionTypeInfo & func, ti.cfSignals) {
			if (isFirst) isFirst = false;
			else         js << ",\n";
			js << '\t' << func.name << ": new __RSig(" << objName << ", '" << func.name << "', '" <<
				ti.typeName.toLower() << "', function(";
			if (func.parameters.isEmpty()) {
				js << ") {})";
			} else {
				js << "__S, " << getJSParameters(func, false) << ") { __S" << getSerializeJSParameters(func) << "; })";
			}
		}
		js << "\n"
			"};\n"
			"__rmi.registerRSig('" << ti.typeName.toLower() << "', function(__D) {\n"
			"\tvar __rsig = __D.s();\n";
		isFirst = true;
		foreach (const SerializeFunctionTypeInfo & func, ti.cfSignals) {
			js << '\t';
			if (isFirst) isFirst = false;
			else         js << "else ";
			js << "if (__rsig == '" << func.name << "') " << objName << ".rsig." << func.name << ".fire(";
			bool isFirst2 = true;
			foreach (const SerializeVariableTypeInfo & p, func.parameters) {
				if (isFirst2) isFirst2 = false;
				else          js << ", ";
				js << getDeserializeCode(p.type, false);
			}
			js << ");\n";
		}
		js <<
			"});\n"
			"\n";
	}

	foreach (const SerializeFunctionTypeInfo & func, ti.functions) {
		const bool hasRV = func.hasReturnValues();
		if (func.parameters.isEmpty()) {
			js << objName << '.' << func.name << " = function(" << (hasRV ? "callback, context" : "") << ") {\n"
				"\t__rmi.send" << (hasRV ? "Request" : "Async") << "(__ber.S().s('"
				<< ti.typeName.toLower() << "').s('" << func.signature() << "').box(2)";
		} else {
			js << objName << '.' << getJSFunctionName(containerRE_, func) << " = function("
				<< getJSParameters(func, false) << (hasRV ? ", callback, context" : "") << ") {\n"
				"\t__rmi.send" << (hasRV ? "Request" : "Async") << "(__ber.S().s('"
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
			"\t\tcallback ? function(__data) {\n"
			"\t\t\tvar __D = __ber.D(__data);\n"
			"\t\t\tcallback.call(context";
		if (func.returnType.type != SerializeTypeInfo::Null) {
			js << ", " << getDeserializeCode(func.returnType, false);
		}
		foreach (const SerializeVariableTypeInfo & p, func.parameters) {
			if (!p.isRef) continue;
			js << ", " << getDeserializeCode(p.type, false);
		}
		js <<
			");\n"
			"\t\t} : null);\n"
			"};\n"
			"\n";
	}

	js << "return " << objName << ";\n";
	return js;
}

QString RMIServerBase::generateTSForClass(const SerializeTypeInfo & ti) const
{
	QString base;
	if (!ti.bases.isEmpty()) {
		base = ti.bases[0].getName();
		base.replace("::", "__");
	}

	QString typeName = ti.typeName;
	if (typeName.contains("::")) typeName = typeName.mid(typeName.lastIndexOf("::") + 2);

	QString ts;
	ts << "export abstract class " << typeName << "Dao extends " << (!base.isEmpty() ? base : "__modelBase") << " {\n"
		"\n";

	if (ti.classId != 0) ts << "\tstatic __classId: number = " << QString::number(ti.classId) << ";\n";
	if (!ti.members.isEmpty()) {
		foreach (const SerializeVariableTypeInfo & vti, ti.members) {
			ts << "\t" << formatMembernameForJS(vti) << ": " << getTSTypename(vti.type) << ";\n";
		}
	}
	if (ti.classId != 0 || !ti.members.isEmpty()) ts << "\n";

	ts <<
		"\tconstructor(param?) {\n";
	if (base.isEmpty()) ts << "\t\tsuper(param);\n";
	ts <<
		"\t\tif (param instanceof Uint8Array) {\n"
		"\t\t\tvar __D = __ber.D(param);\n"
		"\t\t\t__D.n();\n";
	if (!base.isEmpty()) ts << "\t\t\tsuper(__D.a());\n";
	foreach (const SerializeVariableTypeInfo & vti, ti.members) {
		ts << "\t\t\tthis." << formatMembernameForJS(vti) << " = " << getDeserializeCode(vti.type, true) << ";\n";
	}

	ts <<
		"\t\t} else {\n";
	if (!base.isEmpty()) ts << "\t\t\tsuper(param);\n";
	ts <<
		"\t\t\tif (!param || typeof param != 'object') param = {};\n";

	foreach (const SerializeVariableTypeInfo & vti, ti.members) {
		const QString name = formatMembernameForJS(vti);
		ts << "\t\t\tthis." << name << " = " << formatJSTypeConstruction(vti.type, "param." + name, true) << ";\n";
	}
	ts <<
		"\t\t}\n"
		"\t}\n"
		"\n"
		"\tprotected __serialize(__S): void {\n"
		"\t\t__S.";
	if (ti.classId != 0) ts << "i(" << typeName << "Dao.__classId)";
	else                 ts << "n()";
	if (!base.isEmpty()) ts << ".o(this, super.__serialize)";
	foreach (const SerializeVariableTypeInfo & vti, ti.members) {
		ts << getSerializeCode(vti.type, "this." + formatMembernameForJS(vti));
	}
	ts << ".data();\n"
		"\t}\n"
		"\n";

	ts <<
		"\tstatic new(model): any {\n";
	if (ti.classId != 0) ts << "\t\t__ber.ClassRegistry.set(" << typeName << "Dao.__classId, model);\n";
	ts <<
		"\t\treturn __ber.dynamicCreate(model);\n"
		"\t}\n"
		"\n"
		"}\n";

	return ts;
}

QString RMIServerBase::generateTSForService(const SerializeTypeInfo & ti) const
{
	QString objName = ti.typeName;
	objName[0] = ti.typeName[0].toLower();
	QString ts;

	foreach (const SerializeFunctionTypeInfo & func, ti.cfSignals) {
		QString funcTypename = func.name;
		funcTypename[0] = func.name[0].toUpper();
		ts << "interface __" << funcTypename << " {\n"
			"\tregister(" << getJSParameters(func.registerParameters, true) << "): Observable<";
		if (func.parameters.size() > 1) ts << "[";
		bool isFirst = true;
		foreach (const SerializeVariableTypeInfo & p, func.parameters) {
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
		"export class " << ti.typeName << "Api {\n"
		"\n";

	if (!ti.cfSignals.isEmpty()) {
		ts << "\trsig: {\n";
		bool isFirst = true;
		foreach (const SerializeFunctionTypeInfo & func, ti.cfSignals) {
			if (isFirst) isFirst = false;
			else         ts << ",\n";
			QString funcTypename = func.name;
			funcTypename[0] = func.name[0].toUpper();
			ts << "\t\t" << func.name << ": __" << funcTypename;
		}
		ts << "\n"
			"\t};\n"
			"\n"
			"\tconstructor() {\n"
			"\t\tthis.rsig = {\n";

		isFirst = true;
		foreach (const SerializeFunctionTypeInfo & func, ti.cfSignals) {
			if (isFirst) isFirst = false;
			else         ts << ",\n";

			ts << "\t\t\t" << func.name << ": new __RSig(\n"
				"\t\t\t\t'" << ti.typeName.toLower() << "', '" << func.name << "',\n"
				"\t\t\t\tfunction(";
			if (func.registerParameters.isEmpty()) {
				ts << ") {},\n";
			} else {
				ts << "__S, " << getJSParameters(func.registerParameters, false) << ") {\n"
					"\t\t\t\t\t__S" << getSerializeJSParameters(func.registerParameters) << ";\n"
					"\t\t\t\t},\n";
			}

			ts << "\t\t\t\tfunction(__data) {\n"
				"\t\t\t\t\tvar __D = __ber.D(__data);\n"
				"\t\t\t\t\treturn ";
			if (func.parameters.size() > 1) ts << "[";

			bool isFirst = true;
			foreach (const SerializeVariableTypeInfo & p, func.parameters) {
				if (isFirst) isFirst = false;
				else          ts << ", ";
				ts << getDeserializeCode(p.type, true);
			}

			if (func.parameters.size() > 1) ts << "]";
			ts << ";\n"
				"\t\t\t\t}\n"
				"\t\t\t)";
		}

		ts << "\n"
			"\t\t};\n"
			"\t}\n"
			"\n";
	}

	foreach (const SerializeFunctionTypeInfo & func, ti.functions) {
		const uint rvCount = func.returnValueCount();

		ts << "\t" << func.name << "(" << getJSParameters(func, true) << "): ";
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
			foreach (const SerializeVariableTypeInfo & p, func.parameters) {
				if (!p.isRef) continue;
				if (isFirst) isFirst = false;
				else         ts << ", ";
				ts << getTSTypename(p.type);
			}
			if (rvCount > 1) ts << "]";
			ts << ">";
		}
		ts << " {\n"
			"\t\t" << (rvCount > 0 ? "return " : "") << "__rmi.send" << (rvCount > 0 ? "Request" : "Async") << "(__ber.S().s('"
			<< ti.typeName.toLower() << "').s('" << func.signature() << "')" << getSerializeJSParameters(func) << ".box(2)";

		if (rvCount == 0) {
			ts << ");\n"
				"\t}\n"
				"\n";
			continue;
		}

		ts <<
			",\n"
			"\t\t\tfunction(__data) {\n"
			"\t\t\t\tvar __D = __ber.D(__data);\n"
			"\t\t\t\treturn ";
		if (rvCount > 1) ts << "[";
		bool isFirst = true;
		if (func.returnType.type != SerializeTypeInfo::Null) {
			ts << getDeserializeCode(func.returnType, true);
			isFirst = false;
		}
		foreach (const SerializeVariableTypeInfo & p, func.parameters) {
			if (!p.isRef) continue;
			if (isFirst) isFirst = false;
			else         ts << ", ";
			ts << getDeserializeCode(p.type, true);
		}
		if (rvCount > 1) ts << "]";
		ts << ";\n"
			"\t\t\t});\n"
			"\t}\n"
			"\n";
	}

	ts <<
		"}\n"
		"\n"
		"export const " << objName << "Api: " << ti.typeName << "Api = new " << ti.typeName << "Api();";

	return ts;
}

void RMIServerBase::exportClass(const ClassInfoEl & cl, const QString & path, const QString & dest) const
{
	if (cl.infos.isEmpty()) {
		for (const QString & suffix : QStringList{".js", "dao.ts"}) {
			QString cl = path + suffix;
			QString js = generateJSOrTS(cl);
			QFile f(dest + "/js/" + cl);
			f.open(QFile::WriteOnly | QFile::Truncate);
			f.write(js.toUtf8());
		}
	} else {
		QDir().mkpath(dest + "/js/" + path);
		QString p = path;
		if (!path.isEmpty()) p += '/';
		foreach (const QString & ns, cl.infos.keys()) {
			exportClass(*cl.infos[ns], p + ns, dest);
		}
	}
}

}}}	// namespace

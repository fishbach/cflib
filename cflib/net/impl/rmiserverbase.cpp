/* Copyright (C) 2013-2014 Christian Fischbach <cf@cflib.de>
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

QSet<SerializeTypeInfo> getFunctionClassInfos(const SerializeTypeInfo & ti);

QSet<SerializeTypeInfo> getClassInfos(const SerializeTypeInfo & ti)
{
	QSet<SerializeTypeInfo> retval;
	if (ti.type == SerializeTypeInfo::Class) {
		retval << ti;
		foreach (const SerializeVariableTypeInfo & member, ti.members) {
			retval += getClassInfos(member.type);
		}
		retval += getFunctionClassInfos(ti);
	}
	if (ti.type == SerializeTypeInfo::Class || ti.type == SerializeTypeInfo::Container) {
		foreach (const SerializeTypeInfo & base, ti.bases) {
			retval += getClassInfos(base);
		}
	}
	return retval;
}

QSet<SerializeTypeInfo> getFunctionClassInfos(const SerializeTypeInfo & ti)
{
	QSet<SerializeTypeInfo> retval;
	foreach (const SerializeFunctionTypeInfo & func, ti.functions) {
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

	QStringList retval = types.toList();
	retval.sort();
	return retval;
}

QString formatJSTypeConstruction(const SerializeTypeInfo & ti, const QString & raw)
{
	QString js;
	if (ti.type == SerializeTypeInfo::Class) {
		js << "new " << formatClassnameForJS(ti) << "(" << (raw == "null" ? "" : raw) << ")";
	} else if (ti.type == SerializeTypeInfo::Container) {
		if (ti.typeName.startsWith("Pair<")) {
			if (raw == "null") js << "["
				<< formatJSTypeConstruction(ti.bases[0], "null") << ", "
				<< formatJSTypeConstruction(ti.bases[1], "null") << "]";
			else js << "(!" << raw << " ? ["
				<< formatJSTypeConstruction(ti.bases[0], "null") << ", "
				<< formatJSTypeConstruction(ti.bases[1], "null") << "] : ["
				<< formatJSTypeConstruction(ti.bases[0], raw + "[0]") << ", "
				<< formatJSTypeConstruction(ti.bases[1], raw + "[1]") << "])";
		} else if (ti.typeName.startsWith("List<")) {
			if (raw == "null") js << "[]";
			else js << "__inherit.map(" << raw << ", function(__e) { return "
				<< formatJSTypeConstruction(ti.bases[0], "__e") << "; })";
		} else if (ti.typeName.startsWith("Map<")) {
			if (raw == "null") js << "[]";
			else js << "__inherit.map(" << raw << ", function(__e) { return ["
				<< formatJSTypeConstruction(ti.bases[0], "__e[0]")
				<< ", "
				<< formatJSTypeConstruction(ti.bases[1], "__e[1]") << "]; })";
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

QString getJSParameters(const SerializeFunctionTypeInfo & func)
{
	QString js;
	bool isFirst = true;
	int id = 0;
	foreach (const SerializeVariableTypeInfo & p, func.parameters) {
		if (isFirst) isFirst = false;
		else js << ", ";
		if (p.name.isEmpty()) js << "__param_" << QString::number(++id);
		else js << p.name;
	}
	return js;
}

QString getSerializeCode(const SerializeTypeInfo & ti, const QString & name)
{
	QString js;
	if (ti.type == SerializeTypeInfo::Class) {
		js << ".o(" << name << ")";
	} else if (ti.type == SerializeTypeInfo::Container) {
		if (ti.typeName.startsWith("Pair<")) {
			js	<< ".a(!" << name << " ? null : __ber.S()"
				<< getSerializeCode(ti.bases[0], name + "[0]")
				<< getSerializeCode(ti.bases[1], name + "[1]")
				<< ".data, true)";
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

QString getDeserializeCode(const SerializeTypeInfo & ti)
{
	QString js;
	if (ti.type == SerializeTypeInfo::Class) {
		QString cl = ti.getName();
		cl.replace("::", "__");
		js << "new " << cl << "(__D.a())";
	} else if (ti.type == SerializeTypeInfo::Container) {
		if (ti.typeName.startsWith("Pair<")) {
			js	<< "(function(__data) { var __D = __ber.D(__data); return ["
				<< getDeserializeCode(ti.bases[0]) << ", "
				<< getDeserializeCode(ti.bases[1]) << "]; })(__D.a())";
		} else if (ti.typeName.startsWith("List<")) {
			js	<< "__D.map(function(__D) { return "
				<< getDeserializeCode(ti.bases[0]) << "; })";
		} else if (ti.typeName.startsWith("Map<")) {
			js	<< "__D.map(function(__D) { return ["
				<< getDeserializeCode(ti.bases[0]) << ", "
				<< getDeserializeCode(ti.bases[1]) << "]; })";
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

QString getSerializeJSParameters(const SerializeFunctionTypeInfo & func)
{
	QString js;
	int id = 0;
	foreach (const SerializeVariableTypeInfo & p, func.parameters) {
		QString name = p.name;
		if (name.isEmpty()) name << "__param_" << QString::number(++id);
		js << getSerializeCode(p.type, name);
	}
	return js;
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
		sig.serviceName_ = servInfo.typeName.toLower();
		sig.sigName_ = ti.name;
		sfs.signatures[ti.name] = qMakePair(i, 2);
	}

	foreach (const SerializeTypeInfo & ti, getFunctionClassInfos(servInfo)) {
		ClassInfoEl * ciEl = &classInfos_;
		foreach (const QString & ns, ti.getName().split("::")) {
			ClassInfoEl * & elRef = ciEl->infos[ns.toLower()];
			if (!elRef) elRef = new ClassInfoEl;
			ciEl = elRef;
		}
		ciEl->ti = ti;
	}
}

void RMIServerBase::exportTo(const QString & dest) const
{
	// write services
	QDir().mkpath(dest + "/js/services");
	foreach (const QString & name, services_.keys()) {
		QString service = "services/" + name + ".js";
		QString js = generateJS(service);
		QFile f(dest + "/js/" + service);
		f.open(QFile::WriteOnly | QFile::Truncate);
		f.write(js.toUtf8());
	}

	// write classes (recursive)
	exportClass(classInfos_, "", dest);
}

void RMIServerBase::handleRequest(const Request & request)
{
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
			QString js = generateJS(path.mid(4));
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
		"API-URL: <i>/api/rmi/" << path << "</i><br>\n"
		"JavaScript File: <a href=\"/js/services/" << path << ".js\">/js/services/" << path << ".js</a>\n"
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
		const ClassInfoEl & el = *infoEl.infos[ns];
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

QString RMIServerBase::generateJS(const QString & path) const
{
	if (!path.endsWith(".js")) return QString();
	const SerializeTypeInfo ti = getTypeInfo(path.left(path.length() - 3));
	if (ti.getName().isEmpty()) return QString();

	const bool isService = !ti.functions.isEmpty();
	QString js;
	js <<
		"// ============================================================================\n"
		"// Generated by CFLib\n"
		"// ============================================================================\n"
		"\n"
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
		js << "\t\tthis." << formatMembernameForJS(vti) << " = " << getDeserializeCode(vti.type) << ";\n";
	}
	js <<
		"\t} else {\n";
	if (!base.isEmpty()) js <<"\t\t" << nsPrefix << typeName << ".__super.apply(this, arguments);\n";
	js <<
		"\t\tif (!param || typeof param != 'object') param = {};\n";
	foreach (const SerializeVariableTypeInfo & vti, ti.members) {
		const QString name = formatMembernameForJS(vti);
		js << "\t\tthis." << name << " = " << formatJSTypeConstruction(vti.type, "param." + name) << ";\n";
	}
	js <<
		"\t}\n"
		"};\n"
		<< nsPrefix << typeName << ".prototype.__serialize = function() {\n"
		"\treturn __ber.S()";
	if (!base.isEmpty()) js << ".a(" << base << ".prototype.__serialize.call(this), true)";
	foreach (const SerializeVariableTypeInfo & vti, ti.members) {
		js << getSerializeCode(vti.type, "this." + formatMembernameForJS(vti));
	}
	js << ".data;\n"
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
				js << "__ser, " << getJSParameters(func) << ") { __ser" << getSerializeJSParameters(func) << "; })";
			}
		}
		js << "\n"
			"};\n"
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
				<< getJSParameters(func) << (hasRV ? ", callback, context" : "") << ") {\n"
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
			js << ", " << getDeserializeCode(func.returnType);
		}
		foreach (const SerializeVariableTypeInfo & p, func.parameters) {
			if (!p.isRef) continue;
			js << ", " << getDeserializeCode(p.type);
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

void RMIServerBase::exportClass(const ClassInfoEl & cl, const QString & path, const QString & dest) const
{
	if (cl.infos.isEmpty()) {
		QString cl = path + ".js";
		QString js = generateJS(cl);
		QFile f(dest + "/js/" + cl);
		f.open(QFile::WriteOnly | QFile::Truncate);
		f.write(js.toUtf8());
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

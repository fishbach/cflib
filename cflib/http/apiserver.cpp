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

#include "apiserver.h"

#include <cflib/crypt/util.h>
#include <cflib/http/jsservice.h>
#include <cflib/http/request.h>
#include <cflib/http/uploadservice.h>
#include <cflib/util/log.h>
#include <cflib/util/util.h>

USE_LOG(LogCat::Http)

using namespace cflib::serialize;

namespace cflib { namespace http {

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
		js << "new " << formatClassnameForJS(ti) << "(" << raw << ")";
	} else if (ti.type == SerializeTypeInfo::Container) {
		if (ti.typeName.startsWith("Pair<")) {
			js	<< raw << " === undefined ? [] : ["
				<< formatJSTypeConstruction(ti.bases[0], raw + "[0]") << ", "
				<< formatJSTypeConstruction(ti.bases[1], raw + "[1]") << "]";
		} else if (ti.typeName.startsWith("List<")) {
			js	<< "API.map(" << raw << ", function(el) { return "
				<< formatJSTypeConstruction(ti.bases[0], "el") << "; })";
		} else if (ti.typeName.startsWith("Map<")) {
			js	<< "API.map2(" << raw << ", function(key, val) { return ["
				<< formatJSTypeConstruction(ti.bases[0], "key")
				<< ", "
				<< formatJSTypeConstruction(ti.bases[1], "val") << "]; })";
		}
	} else if (ti.type == SerializeTypeInfo::Basic) {
		if (ti.typeName == "DateTime") {
			js << "(!" << raw << " ? null : new Date(" << raw << "))";
		} else if (ti.typeName == "String" || ti.typeName == "ByteArray") {
			js << "(" << raw << " === undefined ? null : " << raw << ")";
		} else if (ti.typeName.indexOf("int") != -1 || ti.typeName == "tribool") {
			js << "(" << raw << " === undefined ? 0 : " << raw << ")";
		} else if (ti.typeName == "bool") {
			js << "(" << raw << " ? true : false)";
		}
	} else {	// SerializeTypeInfo::Null
		js << "undefined";
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

}

// ============================================================================

ApiServer::ApiServer(bool descriptionEnabled) :
	ThreadVerify("ApiServer", Worker),
	descriptionEnabled_(descriptionEnabled),
	lastId_(0),
	lastExpireCheck_(QDateTime::currentDateTime()),
	containerRE_("^(.+)<(.+)>$")
{
}

ApiServer::~ApiServer()
{
	stopVerifyThread();
}

void ApiServer::registerJSService(JSService * service)
{
	SerializeTypeInfo servInfo = service->getServiceInfo();
	services_[servInfo.typeName.toLower()] = service;
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

void ApiServer::registerUploadService(UploadService * uploadService)
{
	uploadService->setApiServer(this);
	uploadService_[uploadService->getName()] = uploadService;
}

void ApiServer::exportTo(const QString & dest) const
{
	QDir().mkpath(dest + "/js/services");
	foreach (const QString & name, services_.keys()) {
		QString service = "services/" + name + ".js";
		QString js = generateJS(service);
		QFile f(dest + "/js/" + service);
		f.open(QFile::WriteOnly | QFile::Truncate);
		f.write(js.toUtf8());
	}

	exportClass(classInfos_, "", dest);
}

uint ApiServer::getClientId(const QByteArray & clIdData)
{
	SyncedThreadCall<uint> stc(this);
	if (!stc.verify(&ApiServer::getClientId, clIdData)) return stc.retval();

	return clientIds_[clIdData].first;
}

void ApiServer::handleRequest(const Request & request)
{
	logInfo("request from IP %1: %2", request.getRemoteIP(), request.getUrl());

	QString path = request.getUrl().path();

	if (path.startsWith("/api/")) {
		path.remove(0, 5);
		if      (path.startsWith("rmi/"))    doRMI(request, path.mid(4));
		else if (path.startsWith("upload/")) handleUpload(request, path.mid(7));
		else if (descriptionEnabled_ && path.startsWith("services")) showServices(request, path.mid(8));
		else if (descriptionEnabled_ && path.startsWith("classes"))  showClasses(request, path.mid(7));
		else request.sendNotFound();
	} else if (descriptionEnabled_) {
		if (path.startsWith("/js/")) {
			QString js = generateJS(path.mid(4));
			if (!js.isNull()) request.sendText(js, "application/javascript");
		} else if (path == "/api") {
			QString info = HTMLDocHeader;
			info <<
				"<ul>\n"
				"<li><a href=\"api/services\">services</a> - API Services Description</li>\n"
				"<li><a href=\"api/classes\">classes</a> - API Classes Description</li>\n"
				"</ul><ul>\n"
				"<li>rmi/&lt;service&gt; - Service Interface</a></li>\n"
				"</ul>\n";
			request.sendText(info << footer);
		}
	}
}

void ApiServer::showServices(const Request & request, QString path) const
{
	QString info = HTMLDocHeader;

	if (path.isEmpty()) {
		info <<
			"<h3>Services:</h3>\n"
			"<ul>\n";
		foreach (const QString & name, services_.keys()) {
			info
				<< "<li><a href=\"services/" << name << "\">"
				<< services_[name]->getServiceInfo().typeName << "</a></li>\n";
		}
		info <<
			"</ul>\n";

		request.sendText(info << footer);
		return;
	}
	path.remove(0, 1);

	JSService * srv = services_.value(path);
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

	request.sendText(info << footer);
}

void ApiServer::showClasses(const Request & request, QString path) const
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

void ApiServer::classesToHTML(QString & info, const ClassInfoEl & infoEl) const
{
	foreach (const QString & ns, infoEl.infos.keys()) {
		const ClassInfoEl & el = *infoEl.infos[ns];
		if (!el.ti.getName().isEmpty()) {
			QString path = el.ti.getName();
			path.replace("::", "/");
			info << "<li><a href=\"classes/" << path.toLower() << "\">" << el.ti.typeName << "</a></li>\n";
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

QString ApiServer::generateJS(const QString & path) const
{
	if (!path.endsWith(".js")) return QString();
	const SerializeTypeInfo ti = getTypeInfo(path.left(path.length() - 3));
	if (ti.getName().isEmpty()) return QString();

	QString js;
	js <<
		"// ============================================================================\n"
		"// Generated by CFLib\n"
		"// ============================================================================\n"
		"\n"
		"define(['cflib/util/api'";
	foreach (QString type, getMemberTypes(ti)) {
		type.replace("::", "/");
		js << ", '" << type.toLower() << '\'';
	}
	js << "], function(API";
	foreach (QString type, getMemberTypes(ti)) {
		type.replace("::", "__");
		js << ", " << type;
	}
	js << ") {\n"
		"\n";

	if (path.startsWith("services/")) js << generateJSForService(ti);
	else                              js << generateJSForClass(ti);

	js <<
		"\n"
		"});\n";
	return js;
}

SerializeTypeInfo ApiServer::getTypeInfo(const QString & path) const
{
	if (path.startsWith("services/")) {
		JSService * srv = services_[path.mid(9)];
		if (!srv) return SerializeTypeInfo();
		return srv->getServiceInfo();
	}

	const ClassInfoEl * ciEl = &classInfos_;
	foreach (const QString & ns, path.split('/')) {
		ciEl = ciEl->infos[ns];
		if (!ciEl) return SerializeTypeInfo();
	}
	return ciEl->ti;
}

QString ApiServer::generateJSForClass(const SerializeTypeInfo & ti) const
{
	QString base;
	if (!ti.bases.isEmpty()) {
		base = ti.bases[0].getName();
		base.replace("::", "__");
	}

	QString js;

	// JS namespece for debugging
	QString nsPrefix;
	if (!ti.ns.isEmpty()) {
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
		while (--i >= 0) js << '}';
		js << ";\n"
			"\n";
	}

	if (nsPrefix.isEmpty()) js << "var ";
	js <<
		nsPrefix << ti.typeName << " = function() { " <<
		nsPrefix << ti.typeName << ".prototype.constructor.apply(this, arguments); };\n"
		"API.setBase(" << nsPrefix << ti.typeName << ", ";
	if (base.isEmpty()) js << "API.Base";
	else js << base;
	js << ");\n"
		<< nsPrefix << ti.typeName << ".prototype.constructor = function(param) {\n";
	if (base.isEmpty()) js << "\t" << nsPrefix << ti.typeName << ".__super.call(this);\n";
	js <<
		"\tvar initArray = [];\n"
		"\tif (param instanceof Array) initArray = param;\n"
		"\telse if (typeof param == 'object') initArray = [";
	bool isFirst = true;
	if (!base.isEmpty()) {
		isFirst = false;
		js << "new " << base << "(param).__memberArray()";
	}
	foreach (const SerializeVariableTypeInfo & vti, ti.members) {
		if (isFirst) isFirst = false;
		else js << ", ";
		js << "param." << formatMembernameForJS(vti);
	}
	js <<
		"];\n";
	int i = 0;
	if (!base.isEmpty()) {
		js << "\t" << nsPrefix << ti.typeName << ".__super.call(this, initArray[0]);\n";
		++i;
	}
	foreach (const SerializeVariableTypeInfo & vti, ti.members) {
		QString raw;
		raw << "initArray[" << QString::number(i++) << "]";
		js << "\tthis." << formatMembernameForJS(vti) << " = " << formatJSTypeConstruction(vti.type, raw) << ";\n";
	}
	js <<
		"};\n"
		<< nsPrefix << ti.typeName << ".prototype.__memberArray = function() {\n"
		"\treturn [";
	isFirst = true;
	if (!base.isEmpty()) {
		isFirst = false;
		js << base << ".prototype.__memberArray.call(this)";
	}
	foreach (const SerializeVariableTypeInfo & vti, ti.members) {
		if (isFirst) isFirst = false;
		else js << ", ";
		js << "this." << formatMembernameForJS(vti);
	}
	js << "];\n"
		"};\n"
		"return " << nsPrefix << ti.typeName << ";\n";
	return js;
}

QString ApiServer::generateJSForService(const SerializeTypeInfo & ti) const
{
	QString objName = ti.typeName;
	objName[0] = ti.typeName[0].toLower();
	QString js;
	js <<
		"var " << ti.typeName << " = function() {};\n"
		"var " << objName << " = new " << ti.typeName << "();\n"
		"\n";

	foreach (const SerializeFunctionTypeInfo & func, ti.functions) {
		if (func.parameters.isEmpty()) {
			js << objName << '.' << func.name << " = function(callback, context) {\n"
				"\tAPI.rmi('" << ti.typeName.toLower() << "', "
				"['" << func.signature() << "'],";
		} else {
			js << objName << '.' << getJSFunctionName(containerRE_, func) << " = function("
				<< getJSParameters(func) << ", callback, context) {\n"
				"\tAPI.rmi('" << ti.typeName.toLower() << "', "
				"['" << func.signature() << "', " << getJSParameters(func) << "],";
		}

		if (!func.hasReturnValues()) {
			js << " callback";
		} else {
			js << "\n"
				"\t\tfunction(";
			bool isFirst = true;
			if (func.returnType.type != SerializeTypeInfo::Null) {
				isFirst = false;
				js << "__retval";
			}
			int id = 0;
			foreach (const SerializeVariableTypeInfo & p, func.parameters) {
				if (!p.isRef) continue;
				if (isFirst) isFirst = false;
				else js << ", ";
				if (p.name.isEmpty()) js << "__param_" << QString::number(++id);
				else js << p.name;
			}
			js << ") {\n"
				"\t\t\tif (callback) callback.call(this";
			if (func.returnType.type != SerializeTypeInfo::Null) {
				js << ", " << formatJSTypeConstruction(func.returnType, "__retval");
			}
			id = 0;
			foreach (const SerializeVariableTypeInfo & p, func.parameters) {
				if (!p.isRef) continue;
				QString name = p.name;
				if (name.isEmpty()) name = "__param_" + QString::number(++id);
				js << ", " << formatJSTypeConstruction(p.type, name);
			}
			js << ");\n"
				"\t\t}";
		}

		js << ", context);\n"
			"};\n"
			"\n";
	}

	js << "return " << objName << ";\n";
	return js;
}

void ApiServer::doRMI(const Request & request, const QString & path)
{
	if (!services_.contains(path) || !request.isPOST()) return;

	QByteArray requestData = request.getBody();

	const int pos = requestData.indexOf('[');
	if (pos == -1) {
		logWarn("broken RMI request: %1", requestData);
		return;
	}

	if (lastExpireCheck_.secsTo(QDateTime::currentDateTime()) > 60 * 60) checkExpire();

	bool ok = false;
	QByteArray clIdData;
	if (pos == 40) {
		clIdData = requestData.left(pos);
		ok = clientIds_.contains(clIdData);
	}

	QByteArray prependData;
	if (!ok) {
		clIdData = cflib::crypt::randomId();
		prependData = clIdData;
	}
	requestData.remove(0, pos);

	ClientIdTimestamp & el = clientIds_[clIdData];
	if (!ok) {
		el.first = ++lastId_;
		logDebug("new client: %1 -> %2", clIdData, el.first);
	} else {
		logDebug("request from client: %1 -> %2", clIdData, el.first);
	}
	el.second = QDateTime::currentDateTime();

	services_[path]->processServiceJSRequest(requestData, request, el.first, prependData);
}

void ApiServer::checkExpire()
{
	const QDateTime now = QDateTime::currentDateTime();
	QMutableMapIterator<QByteArray, ClientIdTimestamp> it(clientIds_);
	while (it.hasNext()) {
		it.next();
		const ClientIdTimestamp & el = it.value();
		if (el.second.secsTo(now) > 24 * 60 * 60) {
			foreach (JSService * srv, services_.values()) {
				srv->clientExpired(el.first);
			}
			it.remove();
		}
	}
	lastExpireCheck_ = QDateTime::currentDateTime();
}

void ApiServer::exportClass(const ClassInfoEl & cl, const QString & path, const QString & dest) const
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

void ApiServer::handleUpload(const Request & request, QString path) const
{
	UploadService * hdl = uploadService_.value(path);
	if (!hdl || !request.isPOST()) request.sendNotFound();
	else                           hdl->processRequest(request);
}

}}	// namespace

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

#pragma once

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
	void exportTo(const QString & dest) const;
	void handleRequest(const Request & request);
	void sendReply(uint connId, const QByteArray & data);
	QByteArray getRemoteIP(uint connId);

	template<typename C>
	void handleCall(const QByteArray & ba, const quint8 * data, int len, const C & connData, uint connDataId, uint connId)
	{
		if (!verifyThreadCall(&RMIServerBase::handleCall<C>, ba, data, len, connData, connDataId, connId)) return;

		serialize::BERDeserializer deser(ba, data, len);
		uint callNo;
		bool hasReturnValues;
		RMIServiceBase * serviceBase = checkServiceCall(deser, connId, callNo, hasReturnValues);
		if (!serviceBase) return;
		RMIService<C> * service = dynamic_cast<RMIService<C> *>(serviceBase);
		if (service) service->processRMIServiceCall(deser, callNo, hasReturnValues, connData, connDataId, connId);
		else         serviceBase->processRMIServiceCall(deser, callNo, hasReturnValues, connId);
	}

private:
	struct ClassInfoEl;
	class ClassInfos : public QMap<QString, ClassInfoEl *> {
		Q_DISABLE_COPY(ClassInfos)
	public:
		ClassInfos() {}
		~ClassInfos() { foreach (ClassInfoEl * val, values()) delete val; }
	};
	struct ClassInfoEl {
		ClassInfos infos;
		cflib::serialize::SerializeTypeInfo ti;
	};
	struct ServiceFunctions {
		ServiceFunctions() : service(0) {}
		RMIServiceBase * service;
		QMap<QString, QPair<uint, bool> > signatures;
	};

private:
	RMIServiceBase * checkServiceCall(serialize::BERDeserializer & deser, uint connId,
		uint & callNo, bool & hasReturnValues);
	void showServices(const Request & request, QString path) const;
	void showClasses(const Request & request, QString path) const;
	void classesToHTML(QString & info, const ClassInfoEl & infoEl) const;
	QString generateJS(const QString & path) const;
	cflib::serialize::SerializeTypeInfo getTypeInfo(const QString & path) const;
	QString generateJSForClass(const cflib::serialize::SerializeTypeInfo & ti) const;
	QString generateJSForService(const cflib::serialize::SerializeTypeInfo & ti) const;
	void exportClass(const ClassInfoEl & cl, const QString & path, const QString & dest) const;

private:
	WSCommManagerBase & wsService_;
	const QRegularExpression containerRE_;
	QMap<QString, ServiceFunctions> services_;
	ClassInfoEl classInfos_;
	QSet<uint> activeRequests_;
};

}}}	// namespace

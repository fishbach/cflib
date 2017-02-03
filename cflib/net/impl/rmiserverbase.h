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
	void send(uint connId, const QByteArray & data);
	QByteArray getRemoteIP(uint connId);

	template<typename C>
	void handleCall(const QByteArray & ba, const quint8 * data, int len, const C & connData, uint connDataId, uint connId)
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
	void connDataChange(const C & connData, uint connDataId, const QSet<uint> & connIds)
	{
		if (!verifyThreadCall(&RMIServerBase::connDataChange<C>, connData, connDataId, connIds)) return;

		QMapIterator<QString, ServiceFunctions> it(services_);
		while (it.hasNext()) {
			RMIServiceBase * serviceBase = it.next().value().service;
			RMIService<C> * service = dynamic_cast<RMIService<C> *>(serviceBase);
			if (service) service    ->connDataChange(connData, connDataId, connIds);
			else         serviceBase->connDataChange(connIds);
		}
	}

	void connectionClosed(uint connId);

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
		QMap<QString, QPair<uint, uint> > signatures;
	};

private:
	RMIServiceBase * checkServiceCall(serialize::BERDeserializer & deser, uint connId,
		uint & callNo, uint & type);
	void showServices(const Request & request, QString path) const;
	void showClasses(const Request & request, QString path) const;
	void classesToHTML(QString & info, const ClassInfoEl & infoEl) const;
	QString generateJSOrTS(const QString & path) const;
	QString generateJS(const serialize::SerializeTypeInfo & ti) const;
	QString generateTS(const serialize::SerializeTypeInfo & ti) const;
	cflib::serialize::SerializeTypeInfo getTypeInfo(const QString & path) const;
	QString generateJSForClass(const cflib::serialize::SerializeTypeInfo & ti) const;
	QString generateJSForService(const cflib::serialize::SerializeTypeInfo & ti) const;
	QString generateTSForClass(const cflib::serialize::SerializeTypeInfo & ti) const;
	QString generateTSForService(const cflib::serialize::SerializeTypeInfo & ti) const;
	void exportClass(const ClassInfoEl & cl, const QString & path, const QString & dest) const;

private:
	WSCommManagerBase & wsService_;
	const QRegularExpression containerRE_;
	QMap<QString, ServiceFunctions> services_;
	ClassInfoEl classInfos_;
	QSet<uint> activeRequests_;
};

}}}	// namespace

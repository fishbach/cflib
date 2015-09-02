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

#include <cflib/serialize/serializetypeinfo.h>
#include <cflib/util/threadverify.h>

namespace cflib { namespace net {

class RMIServiceBase;
class Request;
class WSCommManagerBase;

namespace impl {

class RMIServerBase : public util::ThreadVerify
{
public:
	RMIServerBase(WSCommManagerBase & wsService);
	~RMIServerBase();

	void registerService(RMIServiceBase * service);
	void exportTo(const QString & dest) const;
	void handleRequest(const Request & request);
	void sendReply(uint connId, const QByteArray & data);
	QByteArray getRemoteIP(uint connId);
	RMIServiceBase * findServiceForCall(const quint8 * & data, int & len, uint connId);

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

private:
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
	QMap<QString, RMIServiceBase *> services_;
	ClassInfoEl classInfos_;
	const QRegularExpression containerRE_;
	QSet<uint> activeRequests_;
};

}}}	// namespace

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

#include <cflib/net/requesthandler.h>
#include <cflib/serialize/serializetypeinfo.h>
#include <cflib/util/threadverify.h>

namespace cflib { namespace util { class EVTimer; }}

namespace cflib { namespace net {

class JSService;
class UploadService;

class ApiServer : public QObject, public RequestHandler, public util::ThreadVerify
{
	Q_OBJECT
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
public:
	ApiServer(bool descriptionEnabled = true);
	~ApiServer();

	void registerService(JSService * service);

	void exportTo(const QString & dest) const;

	void blockExpiration(uint clId, bool noExpiration);

public slots:
	void getClientId(const QByteArray & clIdData, uint & clId) const;

protected:
	virtual void handleRequest(const Request & request);
	virtual void deleteThreadData();

private:
	void init();
	void showServices(const Request & request, QString path) const;
	void showClasses(const Request & request, QString path) const;
	void classesToHTML(QString & info, const ClassInfoEl & infoEl) const;
	QString generateJS(const QString & path) const;
	cflib::serialize::SerializeTypeInfo getTypeInfo(const QString & path) const;
	QString generateJSForClass(const cflib::serialize::SerializeTypeInfo & ti) const;
	QString generateJSForService(const cflib::serialize::SerializeTypeInfo & ti) const;
	void doRMI(const Request & request, const QString & path);
	void checkExpire();
	void exportClass(const ClassInfoEl & cl, const QString & path, const QString & dest) const;

private:
	const bool descriptionEnabled_;
	QMap<QString, JSService *> services_;
	ClassInfoEl classInfos_;
	typedef QPair<uint, QDateTime> ClientIdTimestamp;
	QMap<QByteArray, ClientIdTimestamp> clientIds_;
	QSet<uint> noExpire_;
	uint lastId_;
	const QRegularExpression containerRE_;
	util::EVTimer * expireTimer_;
};

}}	// namespace

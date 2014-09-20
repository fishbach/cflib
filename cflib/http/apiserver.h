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

#include <cflib/http/requesthandler.h>
#include <cflib/serialize/serializetypeinfo.h>

#include <QtCore>

namespace cflib { namespace http {

class JSService;
class UploadHandler;

class ApiServer : public RequestHandler
{
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
	ApiServer();
	~ApiServer();

	void registerService(JSService * service);
	void registerUploadHandler(UploadHandler * uploadHandler);

	void exportTo(const QString & dest) const;

	uint getClientId(const QByteArray & clIdData) const { return clientIds_[clIdData].first; }

protected:
	virtual void handleRequest(const Request & request);

private:
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
	void handleUpload(const Request & request, QString path) const;

private:
	QMap<QString, JSService *> services_;
	QMap<QString, UploadHandler *> uploadHandler_;
	ClassInfoEl classInfos_;
	typedef QPair<uint, QDateTime> ClientIdTimestamp;
	QMap<QByteArray, ClientIdTimestamp> clientIds_;
	uint lastId_;
	QDateTime lastExpireCheck_;
};

}}	// namespace

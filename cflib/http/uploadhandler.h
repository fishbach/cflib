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

#include <cflib/http/request.h>
#include <cflib/util/threadverify.h>

namespace cflib { namespace http {

class ApiServer;

class UploadHandler : public util::ThreadVerify, public PassThroughHandler
{
public:
	UploadHandler(const QString & path, const QString & threadName);

	QString getName() const { return path_; }
	void setApiServer(const ApiServer * apiServer) { apiServer_ = apiServer; }
	virtual void processUploadRequest(const Request & request);
	virtual void morePassThroughData(const Request::Id & id);

protected:
	virtual void handleData(
		const Request::Id & id, uint clientId,
		const QByteArray & name, const QByteArray & filename, const QByteArray & contentType,
		const QByteArray & data, bool isLast) = 0;

private:
	struct RequestData {
		Request request;
		QByteArray boundary;
		QByteArray buffer;
		int state;
		uint clientId;
		QByteArray name;
		QByteArray filename;
		QByteArray contentType;
	};

private:
	void parseMoreData(RequestData & rd);

private:
	const QString path_;
	const ApiServer * apiServer_;
	QHash<Request::Id, RequestData> requests_;
};

}}	// namespace

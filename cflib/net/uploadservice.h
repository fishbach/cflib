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

#include <cflib/net/request.h>
#include <cflib/net/requesthandler.h>
#include <cflib/util/threadverify.h>

namespace cflib { namespace net {

class ApiServer;

class UploadService : public QObject, public RequestHandler, public util::ThreadVerify
{
	Q_OBJECT
public:
	UploadService(const QString & threadName, uint threadCount = 1);
	~UploadService();

signals:
	void getClientId(const QByteArray & clIdData, uint & clId);

protected:
	virtual void handleRequest(const Request & request) = 0;
};

class UploadRequestHandler : public QObject, public util::ThreadVerify, public PassThroughHandler
{
	Q_OBJECT
public:
	UploadRequestHandler(UploadService * uploadHandler, const Request & request);

	virtual void morePassThroughData();

signals:
	void getClientId(const QByteArray & clIdData, uint & clId);

protected:
	virtual bool handleClientId(uint clId);
	virtual bool handleData(const QByteArray & data, bool isLast) = 0;
	virtual void requestEnd();

	const Request request_;
	QByteArray contentType() const { return contentType_; }
	QByteArray name() const { return name_; }
	QByteArray filename() const { return filename_; }

private:
	void init();
	void parseMoreData();

private:
	QByteArray boundary_;
	QByteArray buffer_;
	int state_;
	QByteArray contentType_;
	QByteArray name_;
	QByteArray filename_;
};

}}	// namespace

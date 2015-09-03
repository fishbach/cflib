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
#include <cflib/serialize/serialize.h>
#include <cflib/util/threadverify.h>

namespace cflib { namespace net {

class ApiServer;
class Replier;

class JSService : public util::ThreadVerify
{
public:
	JSService(const QString & threadName, uint threadCount = 1, LoopType loopType = Worker);
	JSService(ThreadVerify * other);

	void processServiceJSRequest(const QByteArray & requestData, const Request & request, uint clId,
		const QByteArray & prependData);

	virtual cflib::serialize::SerializeTypeInfo getServiceInfo() = 0;
	virtual QByteArray processServiceJSCall(const QByteArray & jsData) = 0;
	virtual void clientExpired(uint clId) { Q_UNUSED(clId) }

protected:
	ApiServer & api() const { return *api_; }
	uint clientId() const { return clId_; }
	const Request & request() const { return *requestPtr_; }
	virtual void preCallInit() {}
	Replier delayReply();
	QByteArray getRemoteIP() const;

private:
	ApiServer * api_;
	uint clId_;
	bool delayedReply_;
	const Request * requestPtr_;
	const QByteArray * prependDataPtr_;
	friend class ApiServer;
};

class Replier : public QByteArray
{
public:
	Replier(uint clId, const Request & request, const QByteArray & prependData) :
		clId_(clId), request_(request), prependData_(prependData) {}

	void send();
	uint clientId() const { return clId_; }

private:
	uint clId_;
	Request request_;
	QByteArray prependData_;
	friend class JSService;
};

}}	// namespace

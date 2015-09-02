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

#include <cflib/serialize/serialize.h>
#include <cflib/serialize/serializeber.h>
#include <cflib/util/threadverify.h>

namespace cflib { namespace net {

namespace impl { class RMIServerBase; }
class RMIReplier;

class RMIServiceBase : public util::ThreadVerify
{
public:
	RMIServiceBase(const QString & threadName, uint threadCount = 1, LoopType loopType = Worker);
	RMIServiceBase(ThreadVerify * other);

	void processServiceRequest(const QByteArray & requestData, uint connId);

	virtual cflib::serialize::SerializeTypeInfo getServiceInfo() = 0;
	virtual QByteArray processServiceCall(const QByteArray & data) = 0;

protected:
	uint connId() const { return connId_; }
	RMIReplier delayReply();
	QByteArray getRemoteIP() const;
	virtual void preCallInit() {}

private:
	impl::RMIServerBase * server_;
	uint connId_;
	bool delayedReply_;
	friend class impl::RMIServerBase;
};

template<class C>
class RMIService : public RMIServiceBase
{
};

class RMIReplier : public cflib::serialize::BERSerializer
{
public:
	void send();
	uint connId() const { return connId_; }

private:
	RMIReplier(impl::RMIServerBase & server, uint connId) : server_(server), connId_(connId) {}
	impl::RMIServerBase & server_;
	uint connId_;
	friend class RMIServiceBase;
};

}}	// namespace

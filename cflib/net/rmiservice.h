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

class RMIReplier : public QByteArray
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

class RMIServiceBase : public util::ThreadVerify
{
public:
	virtual cflib::serialize::SerializeTypeInfo getServiceInfo() = 0;

protected:
	RMIServiceBase(const QString & threadName, uint threadCount = 1, LoopType loopType = Worker);
	RMIServiceBase(ThreadVerify * other);

	uint connId() const { return connId_; }
	RMIReplier delayReply();
	QByteArray getRemoteIP() const;
	virtual void preCallInit() {}

protected:
	void processRMIServiceCall(serialize::BERDeserializer deser, uint callNo,  bool hasReturnValues,
		uint connId);
	virtual void processRMIServiceCallImpl(serialize::BERDeserializer & deser, uint callNo) = 0;
	virtual void processRMIServiceCallImpl(serialize::BERDeserializer & deser, uint callNo,
		serialize::BERSerializer & ser) = 0;

private:
	impl::RMIServerBase * server_;
	uint connId_;
	bool delayedReply_;
	friend class impl::RMIServerBase;
};

template<typename C>
class RMIService : public RMIServiceBase
{
protected:
	RMIService(const QString & threadName, uint threadCount = 1, LoopType loopType = Worker) :
		RMIServiceBase(threadName, threadCount, loopType) {}
	RMIService(ThreadVerify * other) : RMIServiceBase(other) {}

	const C & connData() { return connData_; }

private:
	void processRMIServiceCall(serialize::BERDeserializer deser, uint callNo, bool hasReturnValues,
		const C & connData, uint connId)
	{
		if (!verifyThreadCall(&RMIService::processRMIServiceCall, deser, callNo, hasReturnValues,
			connData, connId)) return;

		connData_ = connData;
		RMIServiceBase::processRMIServiceCall(deser, callNo, hasReturnValues, connId);
		connData_ = C();
	}

private:
	C connData_;
	friend class impl::RMIServerBase;
};

}}	// namespace

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

#include <cflib/net/rsig.h>
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

template<typename C> class RMIService;

class RMIServiceBase : public util::ThreadVerify
{
public:
	template<typename F, typename R> class rsig : public cflib::net::RSig<F, R> {};

public:
	virtual cflib::serialize::SerializeTypeInfo getServiceInfo() const = 0;

protected:
	RMIServiceBase(const QString & threadName, uint threadCount = 1, LoopType loopType = Worker);
	RMIServiceBase(ThreadVerify * other);

	inline uint connId() const { return connId_; }
	RMIReplier delayReply();
	QByteArray getRemoteIP() const;
	virtual void preCallInit() {}
	virtual void connectionClosed(bool isLast) { Q_UNUSED(isLast) }

protected:
	virtual void processRMIServiceCallImpl(serialize::BERDeserializer & deser, uint callNo) = 0;
	virtual void processRMIServiceCallImpl(serialize::BERDeserializer & deser, uint callNo,
		serialize::BERSerializer & ser) = 0;
	virtual RSigBase * getCfSignal(uint sigNo) = 0;

private:
	void processRMIServiceCall(serialize::BERDeserializer deser, uint callNo, uint type, uint connId);
	void connectionClosed(uint connId, bool isLast);

private:
	impl::RMIServerBase * server_;
	PerThread<uint> connId_;
	PerThread<bool> delayedReply_;
	friend class impl::RMIServerBase;
	template<typename C> friend class RMIService;
};

template<typename C>
class RMIService : public RMIServiceBase
{
protected:
	RMIService(const QString & threadName, uint threadCount = 1, LoopType loopType = Worker) :
		RMIServiceBase(threadName, threadCount, loopType), connData_(this), connDataId_(this, 0) {}
	RMIService(ThreadVerify * other) : RMIServiceBase(other), connData_(this), connDataId_(this, 0) {}

	inline const C & connData() const { return connData_; }
	inline uint connDataId() const { return connDataId_; }
	virtual void connDataChange() {}

private:
	void processRMIServiceCall(serialize::BERDeserializer deser, uint callNo, uint type,
		const C & connData, uint connDataId, uint connId)
	{
		if (!verifyThreadCall(&RMIService::processRMIServiceCall, deser, callNo, type,
			connData, connDataId, connId)) return;

		connData_ = connData;
		connDataId_ = connDataId;
		RMIServiceBase::processRMIServiceCall(deser, callNo, type, connId);
		connData_ = C();
		connDataId_ = 0;
	}

	void connDataChange(const C & connData, uint connDataId, const QSet<uint> & connIds)
	{
		if (!verifyThreadCall(&RMIService::connDataChange, connData, connDataId, connIds)) return;

		connData_   = connData;
		connDataId_ = connDataId;
		for (uint connId : connIds) {
			connId_ = connId;
			connDataChange();
		}
		connId_     = 0;
		connData_   = C();
		connDataId_ = 0;
	}

	void connectionClosed(const C & connData, uint connDataId, uint connId, bool isLast)
	{
		if (!verifyThreadCall(&RMIService::connectionClosed, connData, connDataId, connId, isLast)) return;

		connData_ = connData;
		connDataId_ = connDataId;
		RMIServiceBase::connectionClosed(connId, isLast);
		connData_ = C();
		connDataId_ = 0;
	}

	using RMIServiceBase::connectionClosed;	// prevent hidden virtual warning

private:
	PerThread<C> connData_;
	PerThread<uint> connDataId_;
	friend class impl::RMIServerBase;
};

}}	// namespace

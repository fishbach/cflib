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

#include <cflib/net/impl/rmiserverbase.h>
#include <cflib/net/requesthandler.h>
#include <cflib/net/rmiservice.h>
#include <cflib/net/wscommmanager.h>

namespace cflib { namespace net {

template<class C>
class RMIServer : public RequestHandler, public WSCommMsgHandler<C>, private impl::RMIServerBase
{
public:
	RMIServer(WSCommManager<C> & commMgr) : RMIServerBase(commMgr) {
		commMgr.registerMsgHandler(3, *this);
	}

	void registerService(RMIService<C> * service) { RMIServerBase::registerService(service); }
	using RMIServerBase::exportTo;

	virtual void handleMsg(quint64,
		const QByteArray & data, int tagLen, int lengthSize, qint32 valueLen,
		const C & connData, uint connId)
	{
		handleCall((const quint8 *)data.constData() + tagLen + lengthSize, valueLen, connData, connId);
	}

protected:
	virtual void handleRequest(const Request & request) { RMIServerBase::handleRequest(request); }

private:
	void handleCall(const quint8 * data, int len, const C & connData, uint connId)
	{
		if (!verifyThreadCall(&RMIServer::handleCall, data, len, connData, connId)) return;
		RMIService<C> * service = (RMIService<C> *)findServiceForCall(data, len, connId);
		if (service) service->processServiceRequest(data, len, connData, connId);
	}
};

}}	// namespace

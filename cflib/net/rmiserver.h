/* Copyright (C) 2013-2017 Christian Fischbach <cf@cflib.de>
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

template<typename C>
class RMIServer :
	public RequestHandler,
	public WSCommMsgHandler<C>,
	public WSCommStateListener<C>,
	private impl::RMIServerBase
{
public:
	RMIServer(WSCommManager<C> & commMgr) : RMIServerBase(commMgr) {
		commMgr.registerMsgHandler(2, *this);
		commMgr.registerStateListener(*this);
	}

	using RMIServerBase::registerService;
	using RMIServerBase::exportTo;

	virtual void handleMsg(quint64,
		const QByteArray & data, int tagLen, int lengthSize, qint32 valueLen,
		const C & connData, uint connDataId, uint connId)
	{
		handleCall(data, (const quint8 *)data.constData() + tagLen + lengthSize, valueLen, connData, connDataId, connId);
	}

	virtual void connDataChange(const C &, const C & newConnData, uint connDataId, const QSet<uint> & connIds)
	{
		RMIServerBase::connDataChange(newConnData, connDataId, connIds);
	}

	virtual void connectionClosed(const C & connData, uint connDataId, uint connId, bool isLast)
	{
		RMIServerBase::connectionClosed(connData, connDataId, connId, isLast);
	}

protected:
	virtual void handleRequest(const Request & request) { RMIServerBase::handleRequest(request); }
};

}}	// namespace

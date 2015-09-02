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
#include <cflib/net/wscommmanager.h>
#include <cflib/net/rmiservice.h>
#include <cflib/serialize/serializetypeinfo.h>

namespace cflib { namespace net {

template<class C>
class RMIServer : public RequestHandler, private impl::RMIServerBase
{
public:
	RMIServer(WSCommManager<C> & commMgr) : RMIServerBase(commMgr) {}

	void registerService(RMIService<C> * service) { RMIServerBase::registerService(service); }
	using RMIServerBase::exportTo;

protected:
	virtual void handleRequest(const Request & request) { RMIServerBase::handleRequest(request); }
	virtual void processServiceRequest(RMIServiceBase * service, const QByteArray & requestData) {
//		((RMIService<C> *)service)->processServiceJSRequest(requestData);
	}
};

}}	// namespace

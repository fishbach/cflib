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

#include "rmiservice.h"

#include <cflib/net/impl/rmiserverbase.h>

namespace cflib { namespace net {

RMIServiceBase::RMIServiceBase(const QString & threadName, uint threadCount, LoopType loopType) :
	util::ThreadVerify(threadName, loopType, threadCount),
	server_(0), connId_(0), delayedReply_(false)
{
}

RMIServiceBase::RMIServiceBase(util::ThreadVerify * other) :
	util::ThreadVerify(other),
	server_(0), connId_(0), delayedReply_(false)
{
}

void RMIServiceBase::processServiceRequest(const quint8 * data, int len, uint connId)
{
	connId_ = connId;
	preCallInit();
	const QByteArray reply = processServiceCall(data, len);
	connId_ = 0;
	if (delayedReply_) {
		delayedReply_ = false;
	} else {
		server_->sendReply(connId_, reply);
	}
}

RMIReplier RMIServiceBase::delayReply()
{
	delayedReply_ = true;
	return RMIReplier(*server_, connId_);
}

QByteArray RMIServiceBase::getRemoteIP() const
{
	return server_->getRemoteIP(connId_);
}

void RMIReplier::send()
{
	server_.sendReply(connId_, data());
}

}}	// namespace

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

#include "jsservice.h"

namespace cflib { namespace http {

JSService::JSService(const QString & threadName) :
	util::ThreadVerify(threadName),
	clId_(0), delayedReply_(false), requestPtr_(0), prependDataPtr_(0)
{
}

void JSService::processServiceJSRequest(const QByteArray & requestData, const Request & request, uint clId,
	const QByteArray & prependData)
{
	if (!verifyThreadCall(&JSService::processServiceJSRequest, requestData, request, clId, prependData)) return;

	clId_           = clId;
	requestPtr_     = &request;
	prependDataPtr_ = &prependData;
	preCallInit(request, clId);
	QByteArray reply = processServiceJSCall(requestData);
	clId_           = 0;
	requestPtr_     = 0;
	prependDataPtr_ = 0;
	if (delayedReply_) {
		delayedReply_ = false;
	} else {
		if (!prependData.isNull()) reply.prepend(prependData);
		request.sendReply(reply, "application/javascript; charset=utf-8");
	}
}

Replier JSService::delayReply()
{
	delayedReply_ = true;
	return Replier(clId_, *requestPtr_, *prependDataPtr_);
}

QHostAddress JSService::getRemoteIP() const
{
	return requestPtr_ ? requestPtr_->getRemoteIP() : QHostAddress();
}

void Replier::send()
{
	prependData_ += data();
	request_.sendReply(prependData_, "application/javascript; charset=utf-8");
}

}}	// namespace

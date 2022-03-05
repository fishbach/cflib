/* Copyright (C) 2013-2022 Christian Fischbach <cf@cflib.de>
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

#include "httpthread.h"

#include <cflib/net/impl/requestparser.h>
#include <cflib/util/util.h>

namespace cflib { namespace net { namespace impl {

HttpThread::HttpThread(uint no, uint count) :
	ThreadVerify(QString("HTTP-Server %1/%2").arg(no).arg(count), util::ThreadVerify::Worker),
	activeRequests_(0),
	shutdown_(false)
{
}

HttpThread::~HttpThread()
{
	waitForRequestsToFinish();
	sem_.acquire();
	stopVerifyThread();
}

void HttpThread::newRequest(TCPConnData * data, const QList<RequestHandler *> & handlers)
{
	++activeRequests_;
	new impl::RequestParser(data, handlers, this);
}

void HttpThread::requestFinished()
{
	--activeRequests_;
	if (shutdown_ && activeRequests_ == 0) sem_.release();
}

void HttpThread::waitForRequestsToFinish()
{
	if (!verifyThreadCall(&HttpThread::waitForRequestsToFinish)) return;

	if (activeRequests_ == 0) sem_.release();
	else shutdown_ = true;
}

}}}	// namespace

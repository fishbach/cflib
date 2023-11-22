/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
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

}}}    // namespace

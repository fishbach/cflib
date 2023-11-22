/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/util/threadverify.h>

namespace cflib { namespace net {

class TCPConnData;
class RequestHandler;

namespace impl {

class HttpThread : public util::ThreadVerify
{
public:
    HttpThread(uint no, uint count);
    ~HttpThread();

    void newRequest(TCPConnData * data, const QList<RequestHandler *> & handlers);
    void requestFinished();

private:
    void waitForRequestsToFinish();

private:
    uint activeRequests_;
    bool shutdown_;
    QSemaphore sem_;
};

}}}    // namespace

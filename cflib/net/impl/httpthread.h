/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/util/threadverify.h>

namespace cflib::net {

class TCPConnData;
class RequestHandler;

namespace impl {

class HttpThread : public util::ThreadVerify
{
public:
    HttpThread(uint no, uint count);
    ~HttpThread();

    void waitForRequestsToFinish();

    void newRequest(TCPConnData * data, const List<RequestHandler *> & handlers);
    void requestFinished();

private:
    void doShutdown();

private:
    uint activeRequests_ = 0;
    bool shutdown_ = false;
    Semaphore sem_;
};

}} // namespace

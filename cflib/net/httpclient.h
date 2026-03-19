/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>

namespace cflib { namespace net {

class TCPManager;

class HttpClient
{
    CF_DISABLE_COPY(HttpClient)
public:
    HttpClient(TCPManager & mgr, bool keepAlive = true);
    ~HttpClient();

    // TODO: getaddrinfo -> dns resolve
    void get(const ByteArray & ip, cfuint16 port, const ByteArray & url);

protected:
    virtual void reply(const ByteArray & raw) = 0;

private:
    TCPManager & mgr_;
    bool keepAlive_;
    class HttpConn;
    HttpConn * conn_;
};

}}    // namespace

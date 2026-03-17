/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base/cfcontainers.h>
#include <cflib/base/macros.h>

namespace cflib { namespace crypt { class TLSCredentials; }}

namespace cflib { namespace net {

class RequestHandler;

class HttpServer
{
    CF_DISABLE_COPY(HttpServer)
public:
    HttpServer(uint threadCount = 2, uint tlsThreadCount = 0);
    ~HttpServer();

    bool start(const CFByteArray & address, cfuint16 port);
    bool start(const CFByteArray & address, cfuint16 port, crypt::TLSCredentials & credentials);
    bool start(int listenSocket);
    bool start(int listenSocket, crypt::TLSCredentials & credentials);
    void stop();
    bool isRunning() const;

    void registerHandler(RequestHandler & handler);

private:
    class Impl;
    Impl * impl_;
};

}}    // namespace

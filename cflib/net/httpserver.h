/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>

namespace cflib::crypt { class TLSCredentials; }

namespace cflib::net {

class RequestHandler;

class HttpServer
{
    CF_DISABLE_COPY(HttpServer)
public:
    HttpServer(uint threadCount = 2, uint tlsThreadCount = 0);
    ~HttpServer();

    bool start(const ByteArray & address, uint16 port);
    bool start(const ByteArray & address, uint16 port, crypt::TLSCredentials & credentials);
    bool start(int listenSocket);
    bool start(int listenSocket, crypt::TLSCredentials & credentials);
    void stop();
    bool isRunning() const;

    void registerHandler(RequestHandler & handler);
    template<typename T>
    void registerHandler(std::unique_ptr<T> & handler) { if (handler) registerHandler(*handler); }

private:
    class Impl;
    Impl * impl_;
};

} // namespace

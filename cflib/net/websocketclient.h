/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>
#include <cflib/crypt/tlscredentials.h>
#include <cflib/util/sig.h>

namespace cflib::util { class ThreadVerify;   }

namespace cflib::net {

class TCPManager;

class WebSocketClient
{
    CF_DISABLE_COPY(WebSocketClient)
public:
    WebSocketClient(const crypt::TLSCredentials & credentials = {}, util::ThreadVerify * other = nullptr);
    ~WebSocketClient();

    void shutdown();

    void connect(const Url & url, const ByteArrayList & headers = {});
    void disconnect();
    bool isConnected() const;

    void send(const ByteArray & data, bool isBinary);

cfsignals:
    sig<void ()> connected;
    sig<void ()> disconnected;
    sig<void (const ByteArray & data, bool isBinary)> receive;

private:
    class Impl;
    Impl * impl_;
};

} // namespace

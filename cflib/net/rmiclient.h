/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/crypt/tlscredentials.h>
#include <cflib/util/sig.h>
#include <cflib/util/threadverify.h>

namespace cflib::net {

class RMIClient : public util::ThreadVerify
{
    CF_DISABLE_COPY(RMIClient)
public:
    RMIClient(const crypt::TLSCredentials & credentials = {}, util::ThreadVerify * other = nullptr);
    ~RMIClient();

    void shutdown();

    void connect(const Url & url, const ByteArrayList & headers = {});
    void disconnect();

    void sendRequest(const ByteArray & data, const std::function<void (const ByteArray &)> & callback);
    void sendAsync(const ByteArray & data, bool doNotBuffer = false);

    void unregisterRSig(uint rsigId);
    uint registerRSig(const String & service, const String & name);

    void registerHandler(uint tagNo, const std::function<void (const ByteArray &)> & func);

    void setAliveTimeoutHandler(uint timeoutMs, const std::function<void (bool timeout)> & func);

cfsignals:
    sig<void ()> connected;
    sig<void ()> disconnected;
    sig<void ()> identityReset;
    sig<void (const ByteArray & data)> messageReceived;
    sig<void (uint rsigId, const ByteArray & params)> rsigReceived;

private:
    class Impl;
    Impl * impl_;
};

} // namespace

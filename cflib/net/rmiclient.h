/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/crypt/tlscredentials.h>
#include <cflib/util/sig.h>

namespace cflib::util { class ThreadVerify; }

namespace cflib::net {

class RSigClientBase;

class RMIClient
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

    size_t nextRSigId();
    void registerRSig(RSigClientBase * rsig);
    void unregisterRSig(uint64 rsigId, const ByteArray & unregData);

    void registerHandler(uint tagNo, const std::function<void (const ByteArray &)> & func);

    void setAliveTimeoutHandler(uint timeoutMs, const std::function<void (bool timeout)> & func);

cfsignals:
    sig<void ()> connected;
    sig<void ()> disconnected;
    sig<void ()> identityReset;

private:
    static void getRegData(const RSigClientBase & rsig, uint64 & id, ByteArray & regData);

private:
    class Impl;
    Impl * impl_;
};

} // namespace

/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "rmiremoteservice.h"

#include <cflib/net/rmiclient.h>

namespace cflib::net {

RMIRemoteCall::RMIRemoteCall(RMIClient & client, const BERSerializer & ser) :
    BERSerializer(ser),
    client_(client)
{
}

void RMIRemoteCall::callAsync()
{
    client_.sendAsync(data());
}

serialize::BERDeserializer RMIRemoteCall::callSync()
{
    Semaphore sem;
    ByteArray reply;
    client_.sendRequest(data(), [&](const ByteArray & r) {
        reply = r;
        sem.release();
    });
    sem.acquire();
    return serialize::BERDeserializer(reply);
}

RMIRemoteService::RMIRemoteService(RMIClient & client, const String & serviceName) :
    client_(client),
    serviceName_(serviceName),
    ser_(serialize::BERSerializer(2) << serviceName)
{
}

size_t RMIRemoteService::nextRSigId()
{
    return client_.nextRSigId();
}

void RMIRemoteService::registerRSig(RSigClientBase * rsig)
{
    client_.registerRSig(rsig);
}

void RMIRemoteService::unregisterRSig(uint64 id, const ByteArray & unregData)
{
    client_.unregisterRSig(id, unregData);
}

} // namespace cflib::net

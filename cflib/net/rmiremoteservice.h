/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/serialize/serializeber.h>

namespace cflib::net {

class RMIClient;

class RMIRemoteCall : public serialize::BERSerializer
{
public:
    RMIRemoteCall(RMIClient & client, const serialize::BERSerializer & ser);

    void callAsync();
    serialize::BERDeserializer callSync();

private:
    RMIClient & client_;
};

class RMIRemoteService
{
public:
    RMIRemoteService(RMIClient & client, const String & serviceName);

    RMIRemoteCall newCall() { return RMIRemoteCall(client_, ser_); }

private:
    RMIClient & client_;
    const String serviceName_;
    const serialize::BERSerializer ser_;
};

} // namespace

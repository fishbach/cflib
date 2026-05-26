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

#include <cflib/base.h>

namespace cflib::net {

class RMIClient;

class RMIRemoteService
{
public:
    RMIRemoteService(RMIClient & client, const String & serviceName);

    RMIClient & rmiClient() { return client_; }
    const RMIClient & rmiClient() const { return client_; }

    inline const String & serviceName() const { return serviceName_; }

private:
    RMIClient & client_;
    const String serviceName_;
};

} // namespace

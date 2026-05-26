/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "rmiremoteservice.h"

namespace cflib::net {

RMIRemoteService::RMIRemoteService(RMIClient & client, const String & serviceName) 
    : client_(client), serviceName_(serviceName)
{
}

} // namespace cflib::net

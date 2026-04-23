/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>

namespace cflib::net {

class RMIRemoteService
{
public:
    RMIRemoteService(const String & serviceName) : serviceName_(serviceName) {}

private:
    const String serviceName_;
};

} // namespace

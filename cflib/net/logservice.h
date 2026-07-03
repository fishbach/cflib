/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/net/rmiservice.h>
#include <cflib/util/log.h>

namespace cflib::net {

class LogService : public RMIService<void>
{
    SERIALIZE_CLASS
public:
    LogService();
    ~LogService();

rmi:
    void log(const String & file, int line, uint16 category, const String & str);
};

} // namespace

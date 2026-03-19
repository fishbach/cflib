/* Copyright (C) 2013-2026 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/base.h>
#include <cflib/net/requesthandler.h>

namespace cflib::net {

class RequestLog : public RequestHandler
{
protected:
    virtual void handleRequest(const Request & request);
};

} // namespace

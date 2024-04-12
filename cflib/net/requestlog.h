/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

#include <cflib/net/requesthandler.h>

#include <QtCore>

namespace cflib { namespace net {

class RequestLog : public RequestHandler
{
protected:
    virtual void handleRequest(const Request & request);
};

}}    // namespace

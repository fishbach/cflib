/* Copyright (C) 2013-2024 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#include "requestlog.h"

#include <cflib/net/request.h>
#include <cflib/util/log.h>

USE_LOG(LogCat::Http)

namespace cflib { namespace net {

void RequestLog::handleRequest(const Request & request)
{
    logInfo("%1 - %2 %3 [%4]", request.getRemoteIP(), request.getMethodName(), request.getUri(), request.getHeader("user-agent"));
}

}}    // namespace

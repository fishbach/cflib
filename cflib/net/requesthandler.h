/* Copyright (C) 2013-2023 Christian Fischbach <cf@cflib.de>
 *
 * This file is part of cflib.
 *
 * Licensed under the MIT License.
 */

#pragma once

namespace cflib { namespace net {

class Request;

class RequestHandler
{
public:
    virtual ~RequestHandler() {}

protected:
    virtual void handleRequest(const Request & request) = 0;
    friend class Request;
};

}}    // namespace
